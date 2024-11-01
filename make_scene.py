import random
import json
import subprocess

EXECUTABLE_PATH = "build/Release/src/raytrace_2"


def run(name: str):
    print("running")
    subprocess.run([EXECUTABLE_PATH, name])
    print("done")
    # process = subprocess.Popen(
    #     [EXECUTABLE_PATH, name], stdout=subprocess.PIPE, text=True
    # )
    # process.communicate()  # Waits for the process to complete
    #
    # if process.returncode != 0:
    #     print(f"Process failed with return code {process.returncode}")
    # else:
    #     print("Process completed successfully")


def get_scene_json_path_str(name: str):
    return f"data/{name}.json"


class Scene:
    def __init__(self) -> None:
        self.materials = []
        self.primitives = []
        self.textures = []
        self.nodes = []
        self.background_color = [0, 0, 0]
        self.camera = {
            "fov": 40,
            "center": [0, 0, 1],
            "look_at": [0, 0, 0],
            "width": 600,
            "aspect_ratio": 1.0,
        }

    def add_lambertian(self, albedo: list[float]):
        return self._add_mat("lambertian", {"albedo": albedo})

    def add_metal(self, albedo: list, fuzz: float):
        return self._add_mat("metal", {"albedo": albedo, "fuzz": fuzz})

    def add_dielectric(self, refraction_idx: float):
        return self._add_mat("dielectric", {"refraction_index": refraction_idx})

    def add_diffuse_light(self, albedo: list[float]):
        return self._add_mat("diffuse_light", {"albedo": albedo})

    def add_texture_mat(self, idx: int):
        return self._add_mat("texture", {"tex_idx": idx})

    def add_noise_tex(self, scale, noise_type, albedo: list = [1, 1, 1]):
        idx = len(self.textures)
        self.textures.append(
            {
                "type": "noise",
                "scale": scale,
                "noise_type": noise_type,
                "albedo": albedo,
            }
        )
        return idx

    def add_node(self, args: dict | None = None, primitive_idx=-1):
        if args is None:
            args = {}
        if primitive_idx != -1:
            args["primitive"] = primitive_idx
        self.nodes.append(args)

    def _add_mat(self, mat_type: str, data: dict):
        new_mat = data.copy()
        new_mat["type"] = mat_type
        idx = len(self.materials)
        self.materials.append(new_mat)
        return idx

    def add_sphere_moving(
        self,
        center: list,
        displacement: list,
        radius: float,
        material: int,
        args: dict | None = None,
    ):
        if args is None:
            args = {}
        args["displacement"] = displacement
        return self.add_sphere(center, radius, material, args)

    def add_sphere(
        self,
        center: list[float],
        radius: float,
        material: int,
        args: dict | None = None,
    ):
        sphere = {
            "type": "sphere",
            "center": center,
            "radius": radius,
            "material": material,
        }
        if args is not None:
            sphere.update(args)
        idx = len(self.primitives)
        self.primitives.append(sphere)
        return idx

    def add_quad(
        self,
        q: list[float],
        u: list[float],
        v: list[float],
        material: int,
        args: dict | None = None,
    ):
        quad = {
            "type": "quad",
            "q": q,
            "u": u,
            "v": v,
            "material": material,
        }
        if args is not None:
            quad.update(args)

        idx = len(self.primitives)
        self.primitives.append(quad)
        return idx

    def add_box(
        self, a: list[float], b: list[float], material: int, args: dict | None = None
    ):
        box = {
            "type": "box",
            "a": a,
            "b": b,
            "material": material,
        }
        if args is not None:
            box.update(args)
        idx = len(self.primitives)
        self.primitives.append(box)
        return idx

    def write_json(self, path):
        with open(path, "w") as json_file:
            json.dump(
                {
                    "textures": self.textures,
                    "materials": self.materials,
                    "primitives": self.primitives,
                    "scene": self.nodes,
                    "camera": self.camera,
                    "background_color": self.background_color,
                },
                json_file,
                indent=2,
            )


def rand_vec3(min, max):
    return [
        random.uniform(min, max),
        random.uniform(min, max),
        random.uniform(min, max),
    ]


def add_floor(scene: Scene):
    boxes_per_side = 20
    ground_mat = scene.add_lambertian([0.48, 0.83, 0.53])
    for i in range(boxes_per_side):
        for j in range(boxes_per_side):
            w = 100.0
            x0 = -1000.0 + i * w
            z0 = -1000.0 + j * w
            y0 = 0.0
            x1 = x0 + w
            y1 = random.uniform(1, 101)
            z1 = z0 + w
            scene.add_box([x0, y0, z0], [x1, y1, z1], ground_mat)


def make_book2_final_scene():
    scene = Scene()

    add_floor(scene)

    scene.add_quad(
        [123, 554, 147], [300, 0, 0], [0, 0, 265], scene.add_diffuse_light([7, 7, 7])
    )

    scene.add_sphere_moving(
        [400, 400, 200], [30, 0, 0], 50, scene.add_lambertian([0.7, 0.3, 0.1])
    )
    dielectric_15 = scene.add_dielectric(1.5)
    scene.add_sphere([260, 150, 45], 50, dielectric_15)
    scene.add_sphere([0, 150, 145], 50, scene.add_metal([0.8, 0.8, 0.9], 1.0))

    const_med = {"density": 0.2, "albedo": [0.2, 0.4, 0.9]}
    scene.add_sphere([360, 150, 145], 70, dielectric_15)
    scene.add_sphere([360, 150, 145], 70, dielectric_15, {"constant_medium": const_med})

    scene.add_sphere(
        [0, 0, 0],
        5000,
        dielectric_15,
        {"constant_medium": {"density": 0.0001, "albedo": [1, 1, 1]}},
    )

    scene.add_sphere(
        [220, 280, 300], 80, scene.add_texture_mat(scene.add_noise_tex(0.2, 1))
    )

    for i in range(len(scene.primitives)):
        scene.add_node(None, i)

    sphere_primitives = []
    white_mat = scene.add_lambertian([0.73, 0.73, 0.73])
    for _ in range(1000):
        sphere_primitives.append(scene.add_sphere(rand_vec3(0, 165), 10, white_mat))

    scene.add_node(
        {
            "transform": {"translation": [-100, 270, 395], "rotation": [15, 0, 1, 0]},
            "children": [{"primitive": i} for i in sphere_primitives],
        }
    )

    return scene


def make_constant_medium(density, albedo: list):
    return {"constant_medium": {"density": density, "albedo": albedo}}


def make_transform(
    translation: list | None = None,
    rotation: list | None = None,
    scale: list | None = None,
):
    ret = {}
    if translation:
        ret["translation"] = translation
    if scale:
        ret["scale"] = scale
    if rotation:
        ret["rotation"] = rotation
    return ret


def add_transform(
    diction: dict,
    translation: list | None = None,
    rotation: list | None = None,
    scale: list | None = None,
):
    diction["transform"] = make_transform(translation, rotation, scale)
    return diction


def add_cornell_box(scene: Scene, big_light: bool = False):
    red = scene.add_lambertian([0.65, 0.05, 0.05])
    white = scene.add_lambertian([0.73, 0.73, 0.73])
    green = scene.add_lambertian([0.12, 0.45, 0.15])
    light = scene.add_diffuse_light([7, 7, 7] if big_light else [15, 15, 15])

    scene.add_node(None, scene.add_quad([555, 0, 0], [0, 555, 0], [0, 0, 555], green))
    scene.add_node(None, scene.add_quad([0, 0, 0], [0, 555, 0], [0, 0, 555], red))
    if big_light:
        light_quad = scene.add_quad([113, 554, 127], [330, 0, 0], [0, 0, 305], light)
    else:
        light_quad = scene.add_quad([343, 554, 332], [-130, 0, 0], [0, 0, -105], light)

    scene.add_node(None, light_quad)
    scene.add_node(None, scene.add_quad([0, 0, 0], [555, 0, 0], [0, 0, 555], white))
    scene.add_node(None, scene.add_quad([0, 555, 0], [555, 0, 0], [0, 0, 555], white))
    scene.add_node(None, scene.add_quad([0, 0, 555], [555, 0, 0], [0, 555, 0], white))


def add_regular_cornell_boxes(scene: Scene):
    white = scene.add_lambertian([0.73, 0.73, 0.73])
    scene.add_node(
        {
            "transform": make_transform([130, 0, 65], [-18, 0, 1, 0]),
            "primitive": scene.add_box([0, 0, 0], [165, 165, 165], white),
        }
    )
    scene.add_node(
        {
            "transform": make_transform([265, 0, 295], [15, 0, 1, 0]),
            "primitive": scene.add_box([0, 0, 0], [165, 330, 165], white),
        }
    )


def add_volume_cornell_boxes(scene: Scene):
    scene.add_node(
        {
            "transform": make_transform([130, 0, 65], [-18, 0, 1, 0]),
            "primitive": scene.add_box(
                [0, 0, 0], [165, 165, 165], 0, make_constant_medium(0.01, [1, 1, 1])
            ),
        }
    )
    scene.add_node(
        {
            "transform": make_transform([265, 0, 295], [15, 0, 1, 0]),
            "primitive": scene.add_box(
                [0, 0, 0], [165, 330, 165], 0, make_constant_medium(0.01, [0, 0, 0])
            ),
        }
    )


def write_json(path: str, data: dict):
    with open(path, "w") as json_file:
        json.dump(data, json_file, indent=2)


def set_cornell_cam(scene: Scene):
    scene.camera["center"] = [278, 278, -800]
    scene.camera["look_at"] = [278, 278, 0]
    scene.camera["fov"] = 40


def run_cornell_box_original_scene(name: str):
    scene = Scene()
    add_cornell_box(scene)
    add_regular_cornell_boxes(scene)
    set_cornell_cam(scene)
    scene.write_json(get_scene_json_path_str(name))
    run(name)


def run_cornell_box_volume_scene(name: str):
    scene = Scene()
    add_volume_cornell_boxes(scene)
    add_cornell_box(scene)
    set_cornell_cam(scene)
    scene.write_json(get_scene_json_path_str(name))
    run(name)


def run_book2_final_scene(name: str):
    scene = make_book2_final_scene()
    scene.camera["center"] = [478, 278, -600]
    scene.camera["look_at"] = [278, 278, 0]
    scene.write_json(get_scene_json_path_str(name))
    run(name)


def main():
    # TODO: flags for settings
    settings = {
        "render_once": True,
        "save_after_render_once": True,
        "num_samples": 1000,
        "max_depth": 50,
        "render_window": True,
    }
    write_json("data/settings.json", settings)
    # run_book2_final_scene("book2_final_scene_10000_samples")
    # run_cornell_box_volume_scene("cornell_volume_10000_samples")
    run_cornell_box_original_scene("test_png_ppm")


if __name__ == "__main__":
    main()
