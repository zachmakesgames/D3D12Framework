# D3D12Framework
A framework intended to make D3D12 apps easier to implement. This framework allows the creation of "render passes" to help organize draw calls and prevent unnecessary context rolls.

To add new geometry to a project:
1. Modify the code in D3D12App.cpp D3D12App::CreateGeometry() to load your own geometry (only supports .obj files for now)
2. Modify the input layout for the PSO in D3D12App.cpp D3D12App::CreatePSOs() to support your geometry if necessary
3. Modify the code in D3D12App.cpp D3D12App::Init() to instantiate your geometry and register it with a render pass


To add new shaders to a project:
1. Modify the code in D3D12App.cpp D3D12App::CreateShaders() to load your own shaders (only supports HLSL for now)
2. Modify the input layout for the PSO in D3D12App.cpp D3D12App::CreatePSOs() to support your shader if necessary
3. Modify or create a new PSO in D3D12App.cpp D3D12App::CreatePSOs() to us your new shader
4. If adding a new shader and not overwriting an old shader, create or modify the specific render pass to use your new shader
5. If adding a new render pass, create and register the render pass in D3D12App.cpp D3d12App::Init()
