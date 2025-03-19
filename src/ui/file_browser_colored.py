#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Colored File Browser for RebelCAD.

This module implements a file browser with color coding using the shared RebelSUITE file color manager.
"""

import os
import logging
import sys
from pathlib import Path

from PyQt5.QtWidgets import (
    QTreeView, QFileSystemModel, QWidget, QVBoxLayout, 
    QHBoxLayout, QToolBar, QAction, QMenu, QMessageBox,
    QInputDialog, QFileDialog, QLineEdit, QLabel, QCompleter,
    QSizePolicy, QAbstractItemView, QStyledItemDelegate
)
from PyQt5.QtCore import (
    Qt, QDir, pyqtSignal, QModelIndex, QSize, QTimer,
    QMimeData, QUrl, QStringListModel, QSortFilterProxyModel
)
from PyQt5.QtGui import (
    QIcon, QCursor, QDrag, QPixmap, QStandardItemModel, 
    QStandardItem, QColor, QBrush, QFont
)

# Import the shared file color manager
# Add the RebelSUITE_Shared_Resources directory to the Python path
shared_resources_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../../RebelSUITE_Shared_Resources'))
if shared_resources_path not in sys.path:
    sys.path.append(shared_resources_path)

from file_color_manager import FileColorManager, ColoredFileSystemProxyModel, ColoredFileItemDelegate

logger = logging.getLogger(__name__)

class CADFileFilterProxyModel(QSortFilterProxyModel, ColoredFileSystemProxyModel):
    """Proxy model for filtering files in the file browser with color coding."""
    
    def __init__(self, parent=None):
        """Initialize the proxy model."""
        QSortFilterProxyModel.__init__(self, parent)
        ColoredFileSystemProxyModel.__init__(self)
        self.setDynamicSortFilter(True)
        self.setFilterCaseSensitivity(Qt.CaseInsensitive)
        self.setSortCaseSensitivity(Qt.CaseInsensitive)
        
    def filterAcceptsRow(self, source_row, source_parent):
        """
        Filter rows based on the filter text.
        
        Args:
            source_row (int): Row in the source model.
            source_parent (QModelIndex): Parent index in the source model.
            
        Returns:
            bool: True if the row should be included, False otherwise.
        """
        # Always show directories
        source_model = self.sourceModel()
        index = source_model.index(source_row, 0, source_parent)
        if source_model.isDir(index):
            return True
            
        # Apply filter to file names
        filter_text = self.filterRegExp().pattern().lower()
        if not filter_text:
            return True
            
        file_name = source_model.fileName(index).lower()
        return filter_text in file_name
        
    def data(self, index, role=Qt.DisplayRole):
        """
        Get data for the given index and role.
        
        Args:
            index (QModelIndex): The index.
            role (int, optional): The role. Defaults to Qt.DisplayRole.
            
        Returns:
            Any: The data for the given index and role.
        """
        # Handle foreground role for coloring
        if role == Qt.ForegroundRole and self.file_color_manager:
            # Map to source model
            source_index = self.mapToSource(index)
            
            # Get the file path
            source_model = self.sourceModel()
            if hasattr(source_model, 'filePath'):
                file_path = source_model.filePath(source_index)
                return self.file_color_manager.get_foreground_role(file_path)
                
        # Call the parent class implementation for other roles
        return QSortFilterProxyModel.data(self, index, role)


class CADFileItemDelegate(QStyledItemDelegate, ColoredFileItemDelegate):
    """Item delegate for displaying colored file items."""
    
    def __init__(self, parent=None, file_color_manager=None):
        """
        Initialize the colored file item delegate.
        
        Args:
            parent (QWidget, optional): The parent widget.
            file_color_manager (FileColorManager, optional): The file color manager.
        """
        QStyledItemDelegate.__init__(self, parent)
        ColoredFileItemDelegate.__init__(self)
        
        if file_color_manager:
            self.set_file_color_manager(file_color_manager)
        
    def initStyleOption(self, option, index):
        """
        Initialize style options for the delegate.
        
        Args:
            option (QStyleOptionViewItem): The style options.
            index (QModelIndex): The index.
        """
        QStyledItemDelegate.initStyleOption(self, option, index)
        
        if self.file_color_manager:
            # Get the file path
            model = index.model()
            if hasattr(model, 'mapToSource'):
                # This is a proxy model, map to source
                source_index = model.mapToSource(index)
                if hasattr(model.sourceModel(), 'filePath'):
                    file_path = model.sourceModel().filePath(source_index)
                    option.palette.setColor(
                        option.palette.Text,
                        self.file_color_manager.get_color(file_path)
                    )


class CADFileBrowser(QWidget):
    """File browser widget for navigating project files with color coding."""
    
    # Signals
    file_selected = pyqtSignal(str)
    directory_changed = pyqtSignal(str)
    file_created = pyqtSignal(str)
    file_deleted = pyqtSignal(str)
    file_renamed = pyqtSignal(str, str)  # old_path, new_path
    
    def __init__(self, parent=None, config=None):
        """
        Initialize the file browser.
        
        Args:
            parent (QWidget, optional): The parent widget.
            config (dict, optional): Configuration settings.
        """
        super().__init__(parent)
        
        self.config = config or {}
        self.current_path = None
        self.file_watcher = None
        self.last_selected_path = None
        
        # Initialize file color manager
        theme = config.get('ui', {}).get('theme', 'dark') if config else 'dark'
        self.file_color_manager = FileColorManager(theme=theme)
        
        # Add CAD-specific special folders
        self.file_color_manager.add_special_folder("models")
        self.file_color_manager.add_special_folder("assemblies")
        self.file_color_manager.add_special_folder("drawings")
        
        self._setup_ui()
        self._connect_signals()
        
        # Set up file system watcher
        self._setup_file_watcher()
        
        # Set up auto-refresh timer
        self.refresh_timer = QTimer(self)
        self.refresh_timer.timeout.connect(self.refresh)
        self.refresh_timer.start(5000)  # Refresh every 5 seconds
        
    def _setup_ui(self):
        """Set up the user interface."""
        # Main layout
        self.layout = QVBoxLayout(self)
        self.layout.setContentsMargins(0, 0, 0, 0)
        self.layout.setSpacing(0)
        
        # Search bar
        self.search_layout = QHBoxLayout()
        self.search_layout.setContentsMargins(5, 5, 5, 5)
        
        self.search_label = QLabel("Filter:")
        self.search_layout.addWidget(self.search_label)
        
        self.search_input = QLineEdit()
        self.search_input.setPlaceholderText("Filter files...")
        self.search_input.setClearButtonEnabled(True)
        self.search_layout.addWidget(self.search_input)
        
        self.layout.addLayout(self.search_layout)
        
        # Toolbar
        self.toolbar = QToolBar()
        self.toolbar.setIconSize(QSize(16, 16))
        self.toolbar.setContentsMargins(0, 0, 0, 0)
        self.layout.addWidget(self.toolbar)
        
        # Create actions
        self.refresh_action = QAction("Refresh", self)
        self.refresh_action.setStatusTip("Refresh the file browser")
        self.toolbar.addAction(self.refresh_action)
        
        self.new_file_action = QAction("New File", self)
        self.new_file_action.setStatusTip("Create a new file")
        self.toolbar.addAction(self.new_file_action)
        
        self.new_folder_action = QAction("New Folder", self)
        self.new_folder_action.setStatusTip("Create a new folder")
        self.toolbar.addAction(self.new_folder_action)
        
        self.toolbar.addSeparator()
        
        self.up_action = QAction("Up", self)
        self.up_action.setStatusTip("Go up one directory")
        self.toolbar.addAction(self.up_action)
        
        self.home_action = QAction("Home", self)
        self.home_action.setStatusTip("Go to home directory")
        self.toolbar.addAction(self.home_action)
        
        # File system model
        self.model = QFileSystemModel()
        self.model.setReadOnly(False)
        self.model.setNameFilterDisables(False)
        
        # Set up file filters based on config
        file_filters = self.config.get("file_browser", {}).get("file_filters", [])
        if file_filters:
            self.model.setNameFilters(file_filters)
        
        # Filter proxy model with color coding
        self.proxy_model = CADFileFilterProxyModel(self)
        self.proxy_model.set_file_color_manager(self.file_color_manager)
        self.proxy_model.setSourceModel(self.model)
        
        # Tree view
        self.tree_view = QTreeView()
        self.tree_view.setModel(self.proxy_model)
        self.tree_view.setAnimated(False)
        self.tree_view.setIndentation(20)
        self.tree_view.setSortingEnabled(True)
        self.tree_view.sortByColumn(0, Qt.AscendingOrder)
        
        # Set custom item delegate for color coding
        self.item_delegate = CADFileItemDelegate(self, self.file_color_manager)
        self.tree_view.setItemDelegate(self.item_delegate)
        
        # Enable drag and drop
        self.tree_view.setDragEnabled(True)
        self.tree_view.setAcceptDrops(True)
        self.tree_view.setDropIndicatorShown(True)
        self.tree_view.setDragDropMode(QAbstractItemView.DragDrop)
        
        # Hide unnecessary columns
        self.tree_view.setHeaderHidden(True)
        for column in range(1, self.model.columnCount()):
            self.tree_view.hideColumn(column)
            
        # Enable context menu
        self.tree_view.setContextMenuPolicy(Qt.CustomContextMenu)
        
        # Set selection mode
        self.tree_view.setSelectionMode(QAbstractItemView.ExtendedSelection)
        
        self.layout.addWidget(self.tree_view)
        
        # Status bar
        self.status_bar = QLabel()
        self.status_bar.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.status_bar.setContentsMargins(5, 2, 5, 2)
        self.layout.addWidget(self.status_bar)
        
    def _setup_file_watcher(self):
        """Set up file system watcher to monitor changes."""
        from PyQt5.QtCore import QFileSystemWatcher
        
        self.file_watcher = QFileSystemWatcher(self)
        self.file_watcher.directoryChanged.connect(self._on_directory_changed)
        self.file_watcher.fileChanged.connect(self._on_file_changed)
        
    def _connect_signals(self):
        """Connect signals to slots."""
        # Tree view signals
        self.tree_view.doubleClicked.connect(self._on_item_double_clicked)
        self.tree_view.customContextMenuRequested.connect(self._show_context_menu)
        self.tree_view.selectionModel().selectionChanged.connect(self._on_selection_changed)
        
        # Toolbar actions
        self.refresh_action.triggered.connect(self.refresh)
        self.new_file_action.triggered.connect(self._create_new_file)
        self.new_folder_action.triggered.connect(self._create_new_folder)
        self.up_action.triggered.connect(self._go_up)
        self.home_action.triggered.connect(self._go_home)
        
        # Search input
        self.search_input.textChanged.connect(self._on_search_text_changed)
        
    def set_root_path(self, path):
        """
        Set the root path for the file browser.
        
        Args:
            path (str): The root path to display.
        """
        if not os.path.exists(path):
            logger.error(f"Path does not exist: {path}")
            return
            
        self.current_path = path
        
        # Set the root path in the model
        root_index = self.model.setRootPath(path)
        proxy_index = self.proxy_model.mapFromSource(root_index)
        self.tree_view.setRootIndex(proxy_index)
        
        # Expand the root item
        self.tree_view.expand(proxy_index)
        
        # Add path to file watcher
        if self.file_watcher:
            # Remove old paths
            watched_dirs = self.file_watcher.directories()
            if watched_dirs:
                self.file_watcher.removePaths(watched_dirs)
                
            # Add new path
            self.file_watcher.addPath(path)
            
            # Add subdirectories
            for root, dirs, _ in os.walk(path):
                for dir_name in dirs:
                    dir_path = os.path.join(root, dir_name)
                    self.file_watcher.addPath(dir_path)
        
        # Update status bar
        self._update_status_bar()
        
        logger.info(f"Set root path: {path}")
        
        # Emit signal
        self.directory_changed.emit(path)
        
    def refresh(self):
        """Refresh the file browser."""
        if self.current_path:
            # Remember the current selection
            selected_indexes = self.tree_view.selectedIndexes()
            selected_paths = []
            
            for index in selected_indexes:
                # Map to source model
                source_index = self.proxy_model.mapToSource(index)
                path = self.model.filePath(source_index)
                selected_paths.append(path)
            
            # Refresh the model
            self.model.setRootPath(self.model.rootPath())
            
            # Restore the selection if possible
            for path in selected_paths:
                index = self.model.index(path)
                if index.isValid():
                    proxy_index = self.proxy_model.mapFromSource(index)
                    self.tree_view.selectionModel().select(proxy_index, Qt.QItemSelectionModel.Select)
            
            # Update status bar
            self._update_status_bar()
            
            logger.info("Refreshed file browser")
            
    def set_file_status(self, file_path, status):
        """
        Set the status for a file.
        
        Args:
            file_path (str): Path to the file.
            status (str): Status ('normal', 'warning', 'error', 'success').
        """
        self.file_color_manager.set_file_status(file_path, status)
        self.tree_view.viewport().update()
        
    def clear_file_status(self, file_path=None):
        """
        Clear the status for a file or all files.
        
        Args:
            file_path (str, optional): Path to the file. If None, clear all statuses.
        """
        self.file_color_manager.clear_file_status(file_path)
        self.tree_view.viewport().update()
        
    def _on_item_double_clicked(self, index):
        """
        Handle double-click on an item.
        
        Args:
            index (QModelIndex): The index of the clicked item.
        """
        # Map to source model
        source_index = self.proxy_model.mapToSource(index)
        path = self.model.filePath(source_index)
        
        if os.path.isfile(path):
            # Emit signal to open the file
            self.file_selected.emit(path)
            logger.info(f"Selected file: {path}")
        elif os.path.isdir(path):
            # Expand or collapse the directory
            if self.tree_view.isExpanded(index):
                self.tree_view.collapse(index)
            else:
                self.tree_view.expand(index)
                
                # Add to file watcher
                if self.file_watcher and not path in self.file_watcher.directories():
                    self.file_watcher.addPath(path)
        
    def _on_selection_changed(self, selected, deselected):
        """
        Handle selection change in the tree view.
        
        Args:
            selected (QItemSelection): The selected items.
            deselected (QItemSelection): The deselected items.
        """
        # Update status bar with selection info
        self._update_status_bar()
        
        # Store last selected path
        indexes = self.tree_view.selectedIndexes()
        if indexes:
            # Map to source model
            source_index = self.proxy_model.mapToSource(indexes[0])
            self.last_selected_path = self.model.filePath(source_index)
        else:
            self.last_selected_path = None
            
    def _on_search_text_changed(self, text):
        """
        Handle search text change.
        
        Args:
            text (str): The new search text.
        """
        # Update filter
        self.proxy_model.setFilterRegExp(text)
        
        # Expand all if searching
        if text:
            self.tree_view.expandAll()
        else:
            self.tree_view.collapseAll()
            
            # Expand root
            root_index = self.tree_view.rootIndex()
            self.tree_view.expand(root_index)
            
        # Update status bar
        self._update_status_bar()
            
    def _on_directory_changed(self, path):
        """
        Handle directory change notification.
        
        Args:
            path (str): The path of the changed directory.
        """
        logger.info(f"Directory changed: {path}")
        
        # Refresh the model
        self.refresh()
        
        # Emit signal
        self.directory_changed.emit(path)
        
    def _on_file_changed(self, path):
        """
        Handle file change notification.
        
        Args:
            path (str): The path of the changed file.
        """
        logger.info(f"File changed: {path}")
        
        # Refresh the model
        self.refresh()
            
    def _show_context_menu(self, position):
        """
        Show the context menu.
        
        Args:
            position (QPoint): The position where the menu should be shown.
        """
        index = self.tree_view.indexAt(position)
        
        if not index.isValid():
            # Clicked on empty space, show default menu
            self._show_default_context_menu(position)
            return
            
        # Map to source model
        source_index = self.proxy_model.mapToSource(index)
        path = self.model.filePath(source_index)
        
        # Create menu
        menu = QMenu()
        
        if os.path.isdir(path):
            # Directory context menu
            open_action = menu.addAction("Open in Explorer")
            open_action.triggered.connect(lambda: self._open_in_explorer(path))
            
            menu.addSeparator()
            
            new_file_action = menu.addAction("New File")
            new_file_action.triggered.connect(lambda: self._create_new_file(path))
            
            new_folder_action = menu.addAction("New Folder")
            new_folder_action.triggered.connect(lambda: self._create_new_folder(path))
            
            menu.addSeparator()
            
            rename_action = menu.addAction("Rename")
            rename_action.triggered.connect(lambda: self._rename_item(path))
            
            delete_action = menu.addAction("Delete")
            delete_action.triggered.connect(lambda: self._delete_item(path))
            
        else:
            # File context menu
            open_action = menu.addAction("Open")
            open_action.triggered.connect(lambda: self.file_selected.emit(path))
            
            open_containing_action = menu.addAction("Open Containing Folder")
            open_containing_action.triggered.connect(lambda: self._open_in_explorer(os.path.dirname(path)))
            
            menu.addSeparator()
            
            # Add status actions for testing
            status_menu = menu.addMenu("Set Status")
            
            normal_action = status_menu.addAction("Normal")
            normal_action.triggered.connect(lambda: self.set_file_status(path, "normal"))
            
            warning_action = status_menu.addAction("Warning")
            warning_action.triggered.connect(lambda: self.set_file_status(path, "warning"))
            
            error_action = status_menu.addAction("Error")
            error_action.triggered.connect(lambda: self.set_file_status(path, "error"))
            
            success_action = status_menu.addAction("Success")
            success_action.triggered.connect(lambda: self.set_file_status(path, "success"))
            
            menu.addSeparator()
            
            rename_action = menu.addAction("Rename")
            rename_action.triggered.connect(lambda: self._rename_item(path))
            
            delete_action = menu.addAction("Delete")
            delete_action.triggered.connect(lambda: self._delete_item(path))
            
        menu.exec_(QCursor.pos())
        
    def _show_default_context_menu(self, position):
        """
        Show the default context menu when clicking on empty space.
        
        Args:
            position (QPoint): The position where the menu should be shown.
        """
        menu = QMenu()
        
        refresh_action = menu.addAction("Refresh")
        refresh_action.triggered.connect(self.refresh)
        
        menu.addSeparator()
        
        new_file_action = menu.addAction("New File")
        new_file_action.triggered.connect(self._create_new_file)
        
        new_folder_action = menu.addAction("New Folder")
        new_folder_action.triggered.connect(self._create_new_folder)
        
        menu.exec_(QCursor.pos())
        
    def _open_in_explorer(self, path):
        """
        Open a path in the system file explorer.
        
        Args:
            path (str): The path to open.
        """
        import subprocess
        import platform
        
        try:
            if platform.system() == "Windows":
                os.startfile(path)
            elif platform.system() == "Darwin":  # macOS
                subprocess.call(["open", path])
            else:  # Linux
                subprocess.call(["xdg-open", path])
                
            logger.info(f"Opened in explorer: {path}")
            
        except Exception as e:
            logger.error(f"Error opening path in explorer: {e}", exc_info=True)
            QMessageBox.critical(
                self,
                "Error",
                f"Failed to open path in explorer: {path}\n\n{str(e)}"
            )
            
    def _create_new_file(self, parent_path=None):
        """
        Create a new file.
        
        Args:
            parent_path (str, optional): The parent directory path.
                If None, uses the current directory.
        """
        if parent_path is None:
            # Use the current directory
            index = self.tree_view.currentIndex()
            if index.isValid():
                source_index = self.proxy_model.mapToSource(index)
                path = self.model.filePath(source_index)
                if os.path.isfile(path):
                    parent_path = os.path.dirname(path)
                else:
                    parent_path = path
            else:
                parent_path = self.current_path
                
        # Get the file name
        file_name, ok = QInputDialog.getText(
            self,
            "New File",
            "Enter file name:"
        )
        
        if ok and file_name:
            # Create the file
            file_path = os.path.join(parent_path, file_name)
            
            try:
                # Check if the file already exists
                if os.path.exists(file_path):
                    QMessageBox.warning(
                        self,
                        "File Exists",
                        f"The file already exists: {file_path}"
                    )
                    return
                    
                # Create the file
                with open(file_path, 'w') as f:
                    pass
                    
                # Refresh the view
                self.refresh()
                
                # Select the new file
                index = self.model.index(file_path)
                if index.isValid():
                    proxy_index = self.proxy_model.mapFromSource(index)
                    self.tree_view.setCurrentIndex(proxy_index)
                    
                # Emit signal
                self.file_created.emit(file_path)
                    
                logger.info(f"Created new file: {file_path}")
                
            except Exception as e:
                logger.error(f"Error creating file: {e}", exc_info=True)
                QMessageBox.critical(
                    self,
                    "Error",
                    f"Failed to create file: {file_path}\n\n{str(e)}"
                )
                
    def _create_new_folder(self, parent_path=None):
        """
        Create a new folder.
        
        Args:
            parent_path (str, optional): The parent directory path.
                If None, uses the current directory.
        """
        if parent_path is None:
            # Use the current directory
            index = self.tree_view.currentIndex()
            if index.isValid():
                source_index = self.proxy_model.mapToSource(index)
                path = self.model.filePath(source_index)
                if os.path.isfile(path):
                    parent_path = os.path.dirname(path)
                else:
                    parent_path = path
            else:
                parent_path = self.current_path
                
        # Get the folder name
        folder_name, ok = QInputDialog.getText(
            self,
            "New Folder",
            "Enter folder name:"
        )
        
        if ok and folder_name:
            # Create the folder
            folder_path = os.path.join(parent_path, folder_name)
            
            try:
                # Check if the folder already exists
                if os.path.exists(folder_path):
                    QMessageBox.warning(
                        self,
                        "Folder Exists",
                        f"The folder already exists: {folder_path}"
                    )
                    return
                    
                # Create the folder
                os.makedirs(folder_path)
                
                # Refresh the view
                self.refresh()
                
                # Select the new folder
                index = self.model.index(folder_path)
                if index.isValid():
                    proxy_index = self.proxy_model.mapFromSource(index)
                    self.tree_view.setCurrentIndex(proxy_index)
                    
                logger.info(f"Created new folder: {folder_path}")
                
            except Exception as e:
                logger.error(f"Error creating folder: {e}", exc_info=True)
                QMessageBox.critical(
                    self,
                    "Error",
                    f"Failed to create folder: {folder_path}\n\n{str(e)}"
                )
                
    def _rename_item(self, path):
        """
        Rename a file or folder.
        
        Args:
            path (str): The path to rename.
        """
        # Get the current name
        current_name = os.path.basename(path)
        
        # Get the new name
        new_name, ok = QInputDialog.getText(
            self,
            "Rename",
            "Enter new name:",
            text=current_name
        )
        
        if ok and new_name and new_name != current_name:
            # Create the new path
            new_path = os.path.join(os.path.dirname(path), new_name)
            
            try:
                # Check if the new path already exists
                if os.path.exists(new_path):
                    QMessageBox.warning(
                        self,
                        "Path Exists",
                        f"The path already exists: {new_path}"
                    )
                    return
                    
                # Rename the path
                os.rename(path, new_path)
                
                # Refresh the view
                self.refresh()
                
                # Select the renamed item
                index = self.model.index(new_path)
                if index.isValid():
                    proxy_index = self.proxy_model.mapFromSource(index)
                    self.tree_view.setCurrentIndex(proxy_index)
                    
                # Emit signal
                self.file_renamed.emit(path, new_path)
                    
                logger.info(f"Renamed {path} to {new_path}")
                
            except Exception as e:
                logger.error(f"Error renaming path: {e}", exc_info=True)
                QMessageBox.critical(
                    self,
                    "Error",
                    f"Failed to rename path: {path}\n\n{str(e)}"
                )
                
    def _delete_item(self, path):
        """
        Delete a file or folder.
        
        Args:
            path (str): The path to delete.
        """
        # Confirm deletion
        if os.path.isdir(path):
            message = f"Are you sure you want to delete the folder and all its contents?\n\n{path}"
        else:
            message = f"Are you sure you want to delete the file?\n\n{path}"
            
        reply = QMessageBox.question(
            self,
            "Confirm Delete",
            message,
            QMessageBox.Yes | QMessageBox.No,
            QMessageBox.No
        )
        
        if reply == QMessageBox.Yes:
            try:
                if os.path.isdir(path):
                    import shutil
                    shutil.rmtree(path)
                else:
                    os.remove(path)
                    
                # Refresh the view
                self.refresh()
                
                # Emit signal
                self.file_deleted.emit(path)
                    
                logger.info(f"Deleted {path}")
                
            except Exception as e:
                logger.error(f"Error deleting path: {e}", exc_info=True)
                QMessageBox.critical(
                    self,
                    "Error",
                    f"Failed to delete path: {path}\n\n{str(e)}"
                )
                
    def _update_status_bar(self):
        """Update the status bar with current information."""
        if not hasattr(self, 'status_bar'):
            return
            
        # Get selected items
        selected_indexes = self.tree_view.selectedIndexes()
        
        if not selected_indexes:
            # No selection, show directory info
            if self.current_path:
                try:
                    # Count files and folders
                    file_count = 0
                    folder_count = 0
                    
                    for item in os.listdir(self.current_path):
                        item_path = os.path.join(self.current_path, item)
                        if os.path.isdir(item_path):
                            folder_count += 1
                        else:
                            file_count += 1
                            
                    self.status_bar.setText(f"{folder_count} folders, {file_count} files")
                except Exception:
                    self.status_bar.setText("")
            else:
                self.status_bar.setText("")
        else:
            # Show selection info
            file_count = 0
            folder_count = 0
            total_size = 0
            
            for index in selected_indexes:
                # Map to source model
                source_index = self.proxy_model.mapToSource(index)
                path = self.model.filePath(source_index)
                
                if os.path.isdir(path):
                    folder_count += 1
                else:
                    file_count += 1
                    try:
                        total_size += os.path.getsize(path)
                    except Exception:
                        pass
            
            # Format size
            if total_size < 1024:
                size_str = f"{total_size} bytes"
            elif total_size < 1024 * 1024:
                size_str = f"{total_size / 1024:.1f} KB"
            else:
                size_str = f"{total_size / (1024 * 1024):.1f} MB"
                
            if folder_count > 0 and file_count > 0:
                self.status_bar.setText(f"{folder_count} folders, {file_count} files selected ({size_str})")
            elif folder_count > 0:
                self.status_bar.setText(f"{folder_count} folders selected")
            elif file_count > 0:
                self.status_bar.setText(f"{file_count} files selected ({size_str})")
            else:
                self.status_bar.setText("")
                
    def _go_up(self):
        """Go up one directory level."""
        if self.current_path:
            parent_path = os.path.dirname(self.current_path)
            if os.path.exists(parent_path):
                self.set_root_path(parent_path)
    
    def _go_home(self):
        """Go to the user's home directory."""
        home_path = str(Path.home())
        if os.path.exists(home_path):
            self.set_root_path(home_path)
            
    def set_theme(self, theme):
        """
        Set the theme for the file browser.
        
        Args:
            theme (str): The theme to apply ('dark' or 'light').
        """
        # Update the file color manager theme
        self.file_color_manager.set_theme(theme)
        
        # Refresh the view to apply the new colors
        self.tree_view.viewport().update()
