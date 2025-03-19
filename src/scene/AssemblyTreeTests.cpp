#include "assembly/AssemblyTree.h"
#include <gtest/gtest.h>

using namespace RebelCAD::Assembly;

class AssemblyTreeTest : public ::testing::Test {
protected:
    AssemblyTree tree;
};

TEST_F(AssemblyTreeTest, AddComponent) {
    auto node = tree.addComponent("comp1", "Component 1");
    ASSERT_NE(nullptr, node);
    EXPECT_EQ("comp1", node->getId());
    EXPECT_EQ("Component 1", node->getName());
    EXPECT_TRUE(node->getParent().expired());
    EXPECT_TRUE(node->getChildren().empty());
}

TEST_F(AssemblyTreeTest, AddComponentWithParent) {
    auto parent = tree.addComponent("parent", "Parent");
    auto child = tree.addComponent("child", "Child", "parent");
    
    ASSERT_NE(nullptr, parent);
    ASSERT_NE(nullptr, child);
    EXPECT_EQ(parent, child->getParent().lock());
    EXPECT_EQ(1, parent->getChildren().size());
    EXPECT_EQ(child, parent->getChildren()[0]);
}

TEST_F(AssemblyTreeTest, AddDuplicateId) {
    auto node1 = tree.addComponent("comp1", "Component 1");
    auto node2 = tree.addComponent("comp1", "Component 2");
    ASSERT_NE(nullptr, node1);
    EXPECT_EQ(nullptr, node2);
}

TEST_F(AssemblyTreeTest, RemoveComponent) {
    auto parent = tree.addComponent("parent", "Parent");
    auto child1 = tree.addComponent("child1", "Child 1", "parent");
    auto child2 = tree.addComponent("child2", "Child 2", "parent");

    EXPECT_TRUE(tree.removeComponent("parent"));
    EXPECT_EQ(nullptr, tree.findComponent("parent"));
    
    // Children should be moved to root
    auto movedChild1 = tree.findComponent("child1");
    auto movedChild2 = tree.findComponent("child2");
    ASSERT_NE(nullptr, movedChild1);
    ASSERT_NE(nullptr, movedChild2);
    EXPECT_TRUE(movedChild1->getParent().expired());
    EXPECT_TRUE(movedChild2->getParent().expired());
}

TEST_F(AssemblyTreeTest, MoveComponent) {
    auto parent1 = tree.addComponent("parent1", "Parent 1");
    auto parent2 = tree.addComponent("parent2", "Parent 2");
    auto child = tree.addComponent("child", "Child", "parent1");

    EXPECT_TRUE(tree.moveComponent("child", "parent2"));
    EXPECT_EQ(parent2, child->getParent().lock());
    EXPECT_TRUE(parent1->getChildren().empty());
    EXPECT_EQ(1, parent2->getChildren().size());
}

TEST_F(AssemblyTreeTest, PreventCircularReference) {
    auto parent = tree.addComponent("parent", "Parent");
    auto child = tree.addComponent("child", "Child", "parent");
    auto grandchild = tree.addComponent("grandchild", "Grandchild", "child");

    // Attempt to make parent a child of grandchild
    EXPECT_FALSE(tree.moveComponent("parent", "grandchild"));
    // Verify structure remains unchanged
    EXPECT_TRUE(parent->getParent().expired());
    EXPECT_EQ(parent, child->getParent().lock());
    EXPECT_EQ(child, grandchild->getParent().lock());
}

TEST_F(AssemblyTreeTest, Visibility) {
    auto node = tree.addComponent("comp1", "Component 1");
    EXPECT_TRUE(node->isVisible());
    node->setVisible(false);
    EXPECT_FALSE(node->isVisible());
}

TEST_F(AssemblyTreeTest, Clear) {
    tree.addComponent("comp1", "Component 1");
    tree.addComponent("comp2", "Component 2");
    tree.clear();
    EXPECT_TRUE(tree.isEmpty());
    EXPECT_EQ(0, tree.size());
}

TEST_F(AssemblyTreeTest, Size) {
    EXPECT_EQ(0, tree.size());
    tree.addComponent("comp1", "Component 1");
    EXPECT_EQ(1, tree.size());
    tree.addComponent("comp2", "Component 2", "comp1");
    EXPECT_EQ(2, tree.size());
    tree.removeComponent("comp1");
    EXPECT_EQ(1, tree.size());
}

TEST_F(AssemblyTreeTest, RootComponents) {
    auto root1 = tree.addComponent("root1", "Root 1");
    auto root2 = tree.addComponent("root2", "Root 2");
    tree.addComponent("child", "Child", "root1");

    const auto& roots = tree.getRootComponents();
    EXPECT_EQ(2, roots.size());
    EXPECT_TRUE(std::find(roots.begin(), roots.end(), root1) != roots.end());
    EXPECT_TRUE(std::find(roots.begin(), roots.end(), root2) != roots.end());
}
