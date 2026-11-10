# AGENTS.md

This file provides guidance to coding agents working in this repository.

## Agent Conventions

- **Reasoning language — Chinese required.** All internal reasoning, planning, and intermediate analysis must be written in Chinese (中文). This covers thinking before tool calls, plans, step-by-step derivations, and any scratch notes; it does not require translating source code, identifiers, logs, or quoted file content. Final user-facing replies follow the language the user writes in.

## Project Overview

Unreal Engine **5.8.3** C++ project (`MyProject_5_8`, module of the same name). Content was wiped in commit `32a328e 清理` (2026-10-20), so this is currently a **code sandbox**, not a playable game — do not assume any template content (First Person, Third Person, dungeon pack, demo rooms) still exists.

Active code:

| Path | What it is |
|---|---|
| `Source/MyProject_5_8/FluidTest/SimpleFluidActor.*` | `ASimpleFluidActor` — C++ replica of the NinjaLive fluid actor; component layout mirrors the original Blueprint |
| `Source/MyProject_5_8/FluidTest/MyNinjaLiveComponent.*` | `UMyNinjaLiveComponent` — the fluid simulation component |
| `Source/MyProject_5_8/MyActor.*` | untouched template stub, safe to delete |

Module dependencies: `Core`, `CoreUObject`, `Engine`, `InputCore`, `RHI`. The editor module last built successfully on 2026-07-22.

Rendering config is unchanged: DX12, SM6, Lumen + hardware ray tracing, Substrate materials.

## Broken references left by the cleanup

Check these before trusting editor startup or any automated run:

| Where | Value | Problem |
|---|---|---|
| `Config/DefaultEngine.ini` | `EditorStartupMap` / `GameDefaultMap` = `/Game/M_Start.M_Start` | that map was deleted |
| `Config/DefaultEngine.ini` | `GlobalDefaultGameMode` = `/Script/Engine.GameMode` | engine default — no project game mode |
| `Config/DefaultEditor.ini` | `SimpleMapName` = `/Game/FirstPersonBP/Maps/FirstPersonExampleMap` | deleted |
| `Config/DefaultEditorPerProjectUserSettings.ini` | content browser path `/Game/FirstPersonBP` | deleted |
| `Config/DefaultGame.ini` | `ProjectName = First Person BP Game Template` | stale name |
| `Config/DefaultGame.ini` | `[/Script/McpAutomationBridge.McpAutomationBridgeSettings]` | that plugin is **not installed**; the section is inert |
| `Content/__ExternalActors__/**` | OFPA actor files for `DemoTemplate`, `FirstPerson*`, … | orphaned — their maps are gone; safe to `git rm` |
| C++ comments | `/Game/FluidNinjaLive/NinjaLive` | source art no longer in the repo |

## Content

Only one map survives:

| Path | Purpose |
|---|---|
| `Content/_MyTest/PCG/PCG_Test_02.umap` | PCG test scene — the only `.umap` in the project |
| `Content/Collections`, `Content/Developers` | local editor bookkeeping, not real content |
| `Content/__ExternalActors__`, `Content/__ExternalObjects__` | One File Per Actor storage (mostly orphaned, see above) |

## UE MCP Tools Available

This project has the Unreal MCP server built into UE 5.8+ (the `ModelContextProtocol` plugin, friendly name "Unreal MCP", still Experimental, plugin version 1.0). It does **not** exist in UE 5.6/5.7 — those engines need a third-party MCP server.

The server itself only hosts the protocol; the tools come from Toolset Registry plugins under `Engine\Plugins\Experimental\Toolsets\`. This project enables them individually instead of using the `AllToolsets` aggregator — currently `EditorToolset`, `PCGToolset`, `MetaHumanGenerator`, and `AIAssistant` (see `MyProject_5_8.uproject`). It runs in tool-search mode, so `tools/list` advertises three meta-tools instead of every schema:

- `list_toolsets` — names + descriptions of every registered toolset
- `describe_toolset <name>` — full JSON schema for one toolset
- `call_tool <toolset>.<tool> <args>` — dispatch a call

There is no separate "load" step; describe a toolset, then call its tools. All 23 toolsets registered in this install:

**Editor core** (`editor_toolset.*`)
- **ActorTools** — actors, transforms, labels, parent-child, components
- **AssetTools** — assets in the project and on disk
- **BlueprintTools** — Blueprint inspection/editing
- **ObjectTools** — object/class properties, class discovery
- **SceneTools** — load levels, place/remove actors, viewport camera, outliner
- **PrimitiveTools** — add primitive geometry components
- **MaterialTools** / **MaterialInstanceTools** — materials, expression graphs, MICs
- **StaticMeshTools** / **SkeletalMeshTools** — mesh, materials, bones, sockets
- **TextureTools** — texture assets
- **DataTableTools** / **CurveTableTools** — table assets
- **DataAssetTools** — Data Assets
- **StringTableTools** — string tables
- **ProgrammaticToolset** — sandboxed Python batching calls to other toolsets

**Editor app / diagnostics**
- **EditorAppToolset** — console variables, asset imaging, selection, camera, PIE, content browser
- **LogsToolset** — read output log, set category verbosity
- **AgentSkillToolset** — list/read/create agent skills
- **AIAssistantToolset** — inspect AI Assistant editor state

**Domain toolsets**
- **PCGToolset** / **PCGSpatialToolset** — build and modify PCG graphs
- **MetaHumanToolset** — create/edit MetaHuman characters

Misconceptions to avoid: there is no `load_toolset` tool, and toolset names are dotted paths (e.g. `editor_toolset.toolsets.actor.ActorTools`), not bare class names. After authoring a new toolset, run `ModelContextProtocol.RefreshTools` in the editor console and reconnect the MCP client.

### MCP routes available in this repo

| Route | Endpoint | Notes |
|---|---|---|
| Built-in server | `http://127.0.0.1:8000/mcp` | `ModelContextProtocolSettings.Server=True` in Saved config, so it auto-starts |
| `Tools/FaceMigration/Scripts/ue-mcp.ps1` | same endpoint | dependency-free PowerShell client (`-Toolsets`, `-Describe`, `-Toolset` / `-Tool`) |
| `.claude/settings.local.json` | `127.0.0.1:8091` | launches third-party `npx unreal-engine-mcp-server`; unrelated to the built-in server |

`Tools/FaceMigration/Scripts/ue-mcp.ps1` is a **copy** — the canonical version lives in the DSH repo at `_mytools\Mode\dsh-unreal\tools\ue-mcp.ps1`. Edit that one and re-copy.

## Common Tasks

### Launch the project in UE Editor
```
"E:\Unreal\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "E:\Unreal\Projects\MyProject_5_8\MyProject_5_8.uproject"
```

### Build the editor target
```
"E:\Unreal\UE_5.8\Engine\Build\BatchFiles\Build.bat" MyProject_5_8Editor Win64 Development -Project="E:\Unreal\Projects\MyProject_5_8\MyProject_5_8.uproject" -WaitMutex
```

### Launch Codex with this project
```
Codex --permission-mode bypassPermissions
```
There is no `ClaudeStart.bat` in the repo root. To regenerate the MCP client config, run `ModelContextProtocol.GenerateClientConfig Codex` (or `All`) in the editor console — it writes `.mcp.json` into the project root.

### Run Python
There is no `Content/Python` anymore (deleted with the content). Use the editor's Python console, or use `Tools/FaceMigration/migrate.py` and reusable scripts under `Tools/FaceMigration/Scripts/`; temporary probes belong in `Saved/FaceMigration/`.

### Generate Visual Studio project files
Right-click `.uproject` > "Generate Visual Studio project files", or run `UnrealBuildTool -projectfiles`.

## Key Config

- **Engine**: UE 5.8.3, `E:\Unreal\UE_5.8` (CL 58210709); other installs live under `D:\Unreal\` / `E:\Unreal\`
- **Module**: `MyProject_5_8` (Runtime), deps `Core` / `CoreUObject` / `Engine` / `InputCore` / `RHI`
- **Default map / GameMode**: currently point at deleted assets — see "Broken references" above
- **Custom collision channel**: `Projectile` (`ECC_GameTraceChannel1`); the preset and `DefaultChannelResponses` entry still exist in `DefaultEngine.ini`
- **Input**: the Enhanced Input mapping contexts lived in `Content/Input` and were deleted with the rest of the content
- **MCP auto-start**: enabled (`ModelContextProtocolSettings.Server=True`)
- **Graphics**: DX12, SM6, Lumen + HWRT, ray tracing enabled, Substrate materials

## Plugins

- `ModelContextProtocol` — Epic's built-in UE MCP server (enables AI tools in-editor)
- `AllToolsets` — optional aggregator that pulls in every Toolset plugin at once (not enabled here; this project enables individual toolsets instead)
- `EditorToolset` / `PCGToolset` / `MetaHumanGenerator` — the toolset providers currently enabled for this project
- `ToolsetRegistry` — required dependency, collects `UToolsetDefinition` classes and wraps them as MCP tools
- `MCPClientToolset` — lets the toolset registry act as an MCP *client* to other local/private MCP servers (not for custom tool definitions)
- `AIAssistant` — AI assistant panel integration
- `Oceanology_Plugin` — project-local, binary-only ocean/water plugin (`Plugins/Oceanology_Plugin`)
- `WaterAdvanced` / `WaterExtras` — water rendering add-ons
- `MetaHuman` stack — `MetaHuman`, `MetaHumanGenerator`, `MetaHumanCharacter`, `MetaHumanCrowd`, …
- `PCGBiomeCore` / `PCGBiomeSample` — PCG biome graphs
- `BlueprintAssist`, `Terminal`, `ModelingToolsEditorMode`, `Landmass`, `InEditorDocumentation` — editor productivity

## Portable face migration package

The former Docs, Scripts and related ProjectNotes are now inside `Tools/FaceMigration/`.
Start with [package instructions](Tools/FaceMigration/AGENTS.md) and [usage](Tools/FaceMigration/README.md).
Run `python Tools/FaceMigration/migrate.py check` for offline checks. Do not recreate project-root Docs/Scripts or project-specific script copies.
Character samples and regression baselines are preserved under `Tools/FaceMigration/Characters/`; generated reports, probes and backups belong in the target project's `Saved/FaceMigration/`.
