# Gun sprites

| Files | Gun | Source and license |
| --- | --- | --- |
| `blaster_*` | blaster: idle, 8 firing and 15 walking frames | W_K_Studio, [FP Animated Weapons - E11](https://whiteknightstudios.itch.io/fp-animated-weapons-e11) (itch.io), CC0 1.0 |
| `shotgun_*` | double-barrel shotgun: idle, 9 firing and 15 walking frames | W_K_Studio, [FP Animated Weapon "Double Barrel"](https://whiteknightstudios.itch.io/fps) (itch.io), CC BY 4.0 |
| `pistol_*` | service pistol: idle and 5 firing frames | rendered from the 3D model [Service Pistol](https://polyhaven.com/a/service_pistol) (Poly Haven), CC0 1.0 |

The shotgun is under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/): keep this credit
to W_K_Studio with the game when you distribute it. The other two are public domain.

Changes made to the originals:

- Blaster and shotgun: cropped from 512x512 to the area the animations use, brightened
  (gamma 1.9), and every second walking frame kept. The blaster's red muzzle flash is left as
  it is in these files; the game repaints it in the colour of the active band on loading.
  The shotgun's orange flash, and the glow it casts on the barrels, is repainted violet in
  these files, since it is the violet band's gun.
- Pistol: the model was rendered in Blender (Cycles) from behind and above, barrel pointing
  away, to match the other two guns' view, then cropped. It was thickened a little across its
  length, and the firing frames move its slide back and forward again. The frames show no
  flash; the game draws one.
