import random
import json


def write_json():
    pass


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
        return self._add_mat("dielectric", {"refraction_idx": refraction_idx})

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
        if primitive_idx == -1:
            return
        if args is None:
            args = {}
        args["primitive_idx"] = primitive_idx
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


def make_book2_final_scene():
    boxes_per_side = 2
    scene = Scene()
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

    scene.add_quad(
        [123, 554, 147], [300, 0, 0], [0, 0, 265], scene.add_diffuse_light([7, 7, 7])
    )

    center1 = [400, 400, 200]
    center2 = [center1[0] + 30, center1[1], center1[2]]
    scene.add_sphere_moving(center1, center2, 50, scene.add_lambertian([0.7, 0.3, 0.1]))
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
    for i in range(5):
        sphere_primitives.append(scene.add_sphere(rand_vec3(0, 165), 10, white_mat))

    scene.add_node(
        {
            "transform": {"translation": [-100, 270, 395], "rotation": [15, 0, 1, 0]},
            "children": [{"primitive": i} for i in sphere_primitives],
        }
    )
    scene.camera["center"] = [478, 278, -600]
    scene.camera["look_at"] = [278, 278, 0]
    return scene


def main():
    scene = make_book2_final_scene()
    scene.write_json("data/test.json")


if __name__ == "__main__":
    main()
