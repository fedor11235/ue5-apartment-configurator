"""
Headless generator for the Apartment Configurator demo content.

Run via:
  UnrealEditor-Cmd <project>.uproject -run=pythonscript -script="<this file>"

Creates:
  * /Game/Materials/M_Apartment  — unlit, params "BaseColor" (vector) + "Opacity" (scalar),
    matching what AApartmentActor drives on its dynamic material instance.
  * /Game/Maps/L_Configurator    — lighting, floor, orbit camera controller and one
    AApartmentActor per apartment in Config/BuildingConfig.json (ids + transforms wired up).
"""

import json
import os
import unreal

PROJECT_DIR = unreal.Paths.project_dir()
CONFIG_PATH = os.path.join(PROJECT_DIR, "Config", "BuildingConfig.json")

MATERIAL_PKG = "/Game/Materials"
MATERIAL_NAME = "M_Apartment"
MATERIAL_PATH = MATERIAL_PKG + "/" + MATERIAL_NAME
MAP_PATH = "/Game/Maps/L_Configurator"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_actor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)


def log(msg):
    unreal.log("[gen_configurator] " + msg)


# ---------------------------------------------------------------------------
# Material
# ---------------------------------------------------------------------------
def make_material():
    if unreal.EditorAssetLibrary.does_asset_exist(MATERIAL_PATH):
        unreal.EditorAssetLibrary.delete_asset(MATERIAL_PATH)

    mat = asset_tools.create_asset(MATERIAL_NAME, MATERIAL_PKG, unreal.Material,
                                   unreal.MaterialFactoryNew())
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)

    mel = unreal.MaterialEditingLibrary

    base_color = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -400, 0)
    base_color.set_editor_property("parameter_name", "BaseColor")
    base_color.set_editor_property("default_value", unreal.LinearColor(0.15, 0.55, 0.85, 1.0))

    opacity = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -400, 200)
    opacity.set_editor_property("parameter_name", "Opacity")
    opacity.set_editor_property("default_value", 1.0)

    mel.connect_material_property(base_color, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(opacity, "", unreal.MaterialProperty.MP_OPACITY)

    mel.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(MATERIAL_PATH)
    log("Material created: " + MATERIAL_PATH)
    return mat


# ---------------------------------------------------------------------------
# Level
# ---------------------------------------------------------------------------
def basic_shape(name):
    return unreal.load_object(None, "/Engine/BasicShapes/" + name + "." + name)


def make_level(material):
    with open(CONFIG_PATH, "r") as f:
        config = json.load(f)

    level_editor.new_level(MAP_PATH)

    # --- lighting & environment so the scene is not black ---
    editor_actor.spawn_actor_from_class(unreal.DirectionalLight,
                                        unreal.Vector(0, 0, 1000),
                                        unreal.Rotator(-50, -45, 0))
    editor_actor.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 1000))
    editor_actor.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0))

    # --- ground plane ---
    floor = editor_actor.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, 0))
    floor.set_actor_label("Ground")
    floor_mesh = floor.static_mesh_component
    floor_mesh.set_static_mesh(basic_shape("Plane"))
    floor.set_actor_scale3d(unreal.Vector(40, 40, 1))

    # --- orbit camera controller ---
    cam = editor_actor.spawn_actor_from_class(unreal.ConfiguratorCameraController, unreal.Vector(0, 0, 0))
    cam.set_actor_label("ConfiguratorCamera")

    cube = basic_shape("Cube")
    count = 0
    for floor_data in config.get("floors", []):
        for apt in floor_data.get("apartments", []):
            loc = apt.get("focusLocation", {})
            # Actor sits where the apartment's focus points; the camera focus (y=-600) looks at it.
            pos = unreal.Vector(float(loc.get("x", 0.0)), 0.0, float(loc.get("z", 150.0)))
            actor = editor_actor.spawn_actor_from_class(unreal.ApartmentActor, pos)
            actor.set_actor_label("Apt_" + apt.get("id", "unknown"))
            actor.set_editor_property("apartment_id", apt.get("id", ""))
            actor.set_actor_scale3d(unreal.Vector(3.5, 3.5, 2.6))

            mesh_comp = actor.get_component_by_class(unreal.StaticMeshComponent)
            if mesh_comp:
                mesh_comp.set_static_mesh(cube)
                mesh_comp.set_material(0, material)
            count += 1
            log("Placed apartment %s at %s" % (apt.get("id"), pos))

    # --- player start (keeps GameMode happy) ---
    editor_actor.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, -1600, 600))

    level_editor.save_current_level()
    log("Level saved: %s with %d apartment actor(s)." % (MAP_PATH, count))


def main():
    log("Config: " + CONFIG_PATH)
    mat = make_material()
    make_level(mat)
    log("DONE")


main()
