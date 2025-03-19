# Download GLAD using Python pip
Write-Host "Installing GLAD generator..."
python -m pip install glad

# Generate GLAD files with OpenGL 4.5 Core profile and all extensions
Write-Host "Generating GLAD files..."
python -m glad --profile core --api gl=4.5 --generator c --extensions GL_ARB_buffer_storage,GL_ARB_vertex_buffer_object,GL_ARB_vertex_array_object --out-path external/glad

Write-Host "Done generating GLAD files!"
