# ImageAvatar Asset Contract

ImageAvatar is an optional image-based avatar skin for the StackChan Avatar app.
It keeps the official avatar protocol and maps the existing semantic controls
onto generated LVGL image sprites.

Enable it from menuconfig:

```text
StackChan Avatar -> Avatar Skin -> Image Avatar
```

The default selection remains the official `DefaultAvatar`.

For repeatable local builds, use the git-ignored overlay file:

```text
sdkconfig.defaults.local
```

ImageAvatar overlay:

```text
# CONFIG_STACKCHAN_AVATAR_SKIN_DEFAULT is not set
CONFIG_STACKCHAN_AVATAR_SKIN_IMAGE=y
```

After changing the overlay, delete generated `sdkconfig` and rebuild so the
Kconfig choice is regenerated from `sdkconfig.defaults` plus the overlay.

## Current Pack

The bundled pack is generated from:

```text
workspace/stackchan-image-packs/img4635-hatch-pet-stackchan-20260630/final
```

Firmware assets live under:

```text
firmware/main/stackchan/avatar/skins/image/packs/assets/
```

The generated descriptors use LVGL `RGB565A8` data, matching the style of the
official decorator assets.

## Runtime Contract

The pack targets the 320x240 StackChan display and contains:

- one shared 320x240 body layer;
- `neutral`, `happy`, `angry`, `sad`, `doubt`, and `sleepy` expressions;
- four 48x48 left-eye frames per expression;
- four 48x48 right-eye frames per expression;
- four 96x48 mouth frames per expression.

Anchors are stored as offsets from the display center:

```text
left eye:  -20, -36
right eye:  20, -36
mouth:       0, -11
```

## Mapping Rules

Official avatar JSON, modifiers, and app/server control stay unchanged:

- `Emotion` selects the expression set.
- `leftEye.weight` and `rightEye.weight` select eye frames.
- `mouth.weight` selects mouth frames.
- normalized position maps to a small per-part pixel offset.
- visibility, rotation, and size are passed through to the LVGL image object.

Eye frame order in this pack is open to closed, while the official eye weight
semantic is closed to open. The renderer therefore reverses eye weight mapping.
Mouth frame order is closed to open, so mouth weight mapping is direct.

## Build Integration

`main/CMakeLists.txt` keeps the image skin directory out of the default build:

```text
firmware/main/stackchan/avatar/skins/image/
```

Those sources and generated descriptors are compiled only when
`CONFIG_STACKCHAN_AVATAR_SKIN_IMAGE=y`. This preserves the official
`DefaultAvatar` path and prevents unused image assets from increasing the
default firmware size.

## Verification

Host-testable mapping logic is covered by `image_avatar_mapping_test`.

Local ESP-IDF verification on 2026-06-30:

```text
ESP-IDF: v5.5.4
Target: esp32s3
DefaultAvatar build: 0x39adc0 app bytes, 0x155240 bytes free in app partition
ImageAvatar build:  0x476b80 app bytes, 0x079480 bytes free in app partition
Host tests: motion_math_test and image_avatar_mapping_test passed
```

The ImageAvatar build has about 10% free space in the app partition. Treat this
as a budget limit for future image-pack growth: prefer tightly cropped sprites,
avoid full-screen animation frames, and re-check `idf.py build` size output after
every asset-pack update.

Full device verification still requires flashing a physical StackChan:

```bash
get_idf
idf.py build
idf.py flash monitor
```

Minimum device checks:

- Avatar app opens with `DefaultAvatar` when the default skin is selected.
- Avatar app opens with `ImageAvatar` when the image skin is selected.
- speech still updates the speech bubble.
- speaking changes mouth frames.
- emotion changes update image sets for all six emotions.
- app/WebSocket avatar control and dance control still work.
