# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Unreal Engine 5.8 Blueprint-only project based on the First Person BP Game Template. Uses Lumen global illumination, hardware ray tracing, DX12 (SM6), and Substrate materials.

## UE MCP Tools Available

This project has the Unreal MCP server built into UE 5.8 (the `ModelContextProtocol` plugin). Toolsets are auto-loaded from the MCP server. Key toolsets:

- **ActorTools** — inspect/modify actors, transforms, labels, components
- **AssetTools** — interact with assets and files on disk
- **BlueprintTools** — work with Blueprints
- **MaterialTools** — create/edit materials and expression graphs
- **MaterialInstanceTools** — create/modify MaterialInstanceConstants
- **ObjectTools** — inspect/modify Unreal Object/Class properties, discover classes
- **SceneTools** — load levels, place/remove actors, control viewport camera, organize outliner
- **StaticMeshTools** / **SkeletalMeshTools** — inspect/modify mesh assets
- **ProgrammaticToolset** — execute sandboxed Python scripts combining multiple tool calls
- **TextureTools** — work with texture assets
- **DataTableTools** / **CurveTableTools** — create/edit table assets
- **StringTableTools** — create/edit string tables
- **LogsToolset** — read the output log, control log category verbosity
- **EditorAppToolset** — console variables, asset imaging, content browser navigation

To load a toolset: use `load_toolset` with the name from `list_toolsets`, then call its tools on the next turn.

## Common Tasks

### Launch the project in UE Editor
```
"E:\Unreal\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "E:\Unreal\Projects\MyProject_5_8\MyProject_5_8.uproject"
```

### Launch Claude Code with this project
```
claude --permission-mode bypassPermissions
```
(Or use `ClaudeStart.bat` in the project root.)

### Run Python scripts in UE Editor
In the UE Output Log (Window > Developer Tools > Output Log), tab to Python:
```
py "Content/Python/create_dissolve_material.py"
```

### Generate Visual Studio project files
Generate with: right-click `.uproject` > "Generate Visual Studio project files"

## C++ Source Structure

All fluid simulation C++ code is under `Source/MyProject_5_8/FluidTest/`:

| File | Purpose |
|---|---|
| `FluidTest/MyNinjaLiveComponent.h/.cpp` | C++ ActorComponent (UMyNinjaLiveComponent) — GPU fluid simulation pipeline, line-trace interaction, render target management. |
| `FluidTest/SimpleFluidActor.h/.cpp` | Wrapper Actor (ASimpleFluidActor) that owns a UMyNinjaLiveComponent + display plane + volumes. Reference: `/Game/FluidNinjaLive/NinjaLive.NinjaLive`. |
| `MyActor.h/.cpp` | Base actor class |

### Using MyNinjaLiveComponent standalone

Attach it to any Actor in C++ or Blueprint:

```cpp
// C++ — attach to any Actor
UMyNinjaLiveComponent* FluidComp = CreateDefaultSubobject<UMyNinjaLiveComponent>(TEXT("MyNinjaLive"));
FluidComp->AdvectionMat = ...;  // assign materials in constructor
```

In Blueprint, add `MyNinjaLiveComponent` from the "FluidNinja" category.

### Using ASimpleFluidActor (wrapper)

Place `BP_SimpleFluidActor` (or the C++ class directly) in the level. It exposes the key simulation parameters in the Details panel and forwards them to its internal `UMyNinjaLiveComponent`.

## Building

Generate Visual Studio project files: right-click `.uproject` > "Generate Visual Studio project files", then build from VS or compile from the UE Editor.

## Content Structure

| Directory | Purpose |
|---|---|
| `Content/FirstPerson` | First person character BP, weapons, maps |
| `Content/ThirdPerson` | Third person character BP, animations |
| `Content/Characters` | Character-related assets |
| `Content/DemoTemplate` | Demo/template content with localization |
| `Content/Fantastic_Dungeon_Pack` | Marketplace dungeon pack assets |
| `Content/LevelPrototyping` | Prototyping levels |
| `Content/Input` | Input mapping contexts (Enhanced Input) |
| `Content/Python` | Python editor scripts |
| `Content/Localization` | Game localization data |

## Key Config

- **DefaultGameMode**: `/Game/FirstPerson/Blueprints/BP_FirstPersonGameMode.BP_FirstPersonGameMode_C`
- **Default map**: `/Game/M_Start.M_Start`
- **Input system**: Enhanced Input (`EnhancedPlayerInput` / `EnhancedInputComponent`)
- **Custom collision channel**: `Projectile` (ECC_GameTraceChannel1)
- **MCP auto-start**: Enabled in EditorPerProjectUserSettings
- **Graphics**: DX12, SM6, Lumen + HWRT, ray tracing enabled, Substrate materials

## Plugins

- `ModelContextProtocol` — Epic's built-in UE MCP server (enables AI tools in-editor)
- `MCPClientToolset` — companion for registering custom tool definitions
- `ModelingToolsEditorMode` — mesh modeling tools
- `Landmass` — landscape/water brushes
- `InEditorDocumentation` — in-editor doc browser
- `AIAssistant` — AI assistant panel integration
