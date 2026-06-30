# ImageAvatar Asset Contract

ImageAvatar is an optional image-based avatar skin for the StackChan Avatar app.
It keeps the official avatar protocol and maps the existing semantic controls
onto generated LVGL image sprites.

Enable it from menuconfig:

```text
StackChan Avatar -> Avatar Skin -> Image Avatar
```

The default selection remains the official `DefaultAvatar`.

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

## Verification

Host-testable mapping logic is covered by `image_avatar_mapping_test`.

Full firmware verification still requires an ESP-IDF environment and device:

```bash
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
