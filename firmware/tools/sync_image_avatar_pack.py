#!/usr/bin/env python3
"""Sync the bundled ImageAvatar LVGL descriptors from a finalized pack manifest."""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any


EXPECTED_EMOTIONS = ("neutral", "happy", "angry", "sad", "doubt", "sleepy")


def read_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def default_firmware_dir() -> Path:
    return Path(__file__).resolve().parents[1]


def require_file(path: Path) -> Path:
    if not path.is_file():
        raise SystemExit(f"missing file: {path}")
    return path


def manifest_asset_paths(manifest: dict[str, Any], manifest_dir: Path) -> list[tuple[Path, str]]:
    assets: list[tuple[Path, str]] = []
    assets.append((manifest_dir / manifest["body"], "my_stackchan_body_base"))

    emotions = manifest.get("emotions", {})
    missing = [emotion for emotion in EXPECTED_EMOTIONS if emotion not in emotions]
    if missing:
        raise SystemExit(f"manifest is missing emotions: {', '.join(missing)}")

    for emotion in EXPECTED_EMOTIONS:
        entry = emotions[emotion]
        for index, asset in enumerate(entry["leftEye"]):
            assets.append((manifest_dir / asset, f"my_stackchan_{emotion}_left_eye_{index}"))
        for index, asset in enumerate(entry["rightEye"]):
            assets.append((manifest_dir / asset, f"my_stackchan_{emotion}_right_eye_{index}"))
        for index, asset in enumerate(entry["mouth"]):
            assets.append((manifest_dir / asset, f"my_stackchan_{emotion}_mouth_{index}"))

    for path, _name in assets:
        require_file(path)

    return assets


def run_converter(
    *,
    python: str,
    converter: Path,
    source: Path,
    symbol_name: str,
    output_dir: Path,
) -> Path:
    subprocess.run(
        [
            python,
            str(converter),
            "--ofmt",
            "C",
            "--cf",
            "RGB565A8",
            "--name",
            symbol_name,
            "-o",
            str(output_dir),
            str(source),
        ],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    output = output_dir / f"{symbol_name}.c"
    require_file(output)
    output.write_text(output.read_text(encoding="utf-8").rstrip() + "\n", encoding="utf-8")
    return output


def replace_constant(source: str, name: str, value: int) -> str:
    pattern = re.compile(rf"(constexpr int {re.escape(name)}\s*=\s*)-?\d+(;)")
    replacement = rf"\g<1>{value}\2"
    updated, count = pattern.subn(replacement, source, count=1)
    if count != 1:
        raise SystemExit(f"failed to update {name} in pack source")
    return updated


def anchor_offsets(manifest: dict[str, Any]) -> dict[str, int]:
    canvas = manifest["canvas"]
    center_x = int(canvas["width"]) / 2
    center_y = int(canvas["height"]) / 2
    anchors = manifest["anchors"]

    left_eye_y = int(round(float(anchors["leftEye"]["y"]) - center_y))
    right_eye_y = int(round(float(anchors["rightEye"]["y"]) - center_y))
    if left_eye_y != right_eye_y:
        raise SystemExit("firmware pack currently requires left/right eye y anchors to match")

    return {
        "kLeftEyeCenterX": int(round(float(anchors["leftEye"]["x"]) - center_x)),
        "kRightEyeCenterX": int(round(float(anchors["rightEye"]["x"]) - center_x)),
        "kEyeCenterY": left_eye_y,
        "kMouthCenterX": int(round(float(anchors["mouth"]["x"]) - center_x)),
        "kMouthCenterY": int(round(float(anchors["mouth"]["y"]) - center_y)),
    }


def update_pack_source(pack_source: Path, manifest: dict[str, Any]) -> dict[str, int]:
    text = pack_source.read_text(encoding="utf-8")
    offsets = anchor_offsets(manifest)
    updated = text
    for name, value in offsets.items():
        updated = replace_constant(updated, name, value)
    if updated != text:
        pack_source.write_text(updated, encoding="utf-8")
    return offsets


def sync(args: argparse.Namespace) -> dict[str, Any]:
    firmware_dir = Path(args.firmware_dir).expanduser().resolve()
    manifest_path = require_file(Path(args.manifest).expanduser().resolve())
    manifest = read_json(manifest_path)
    manifest_dir = manifest_path.parent

    converter = require_file(Path(args.lvgl_image or firmware_dir / "managed_components/lvgl__lvgl/scripts/LVGLImage.py").resolve())
    asset_dir = Path(args.asset_dir or firmware_dir / "main/stackchan/avatar/skins/image/packs/assets").resolve()
    pack_source = require_file(Path(args.pack_source or firmware_dir / "main/stackchan/avatar/skins/image/packs/my_stackchan_pack.cpp").resolve())

    assets = manifest_asset_paths(manifest, manifest_dir)
    if len(assets) != 73:
        raise SystemExit(f"expected 73 firmware assets, got {len(assets)}")

    asset_dir.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="stackchan-lvgl-") as temp:
        temp_dir = Path(temp)
        generated: list[Path] = []
        for source, symbol_name in assets:
            generated.append(
                run_converter(
                    python=args.python,
                    converter=converter,
                    source=source,
                    symbol_name=symbol_name,
                    output_dir=temp_dir,
                )
            )
        for output in generated:
            shutil.copy2(output, asset_dir / output.name)

    offsets = update_pack_source(pack_source, manifest)
    return {
        "ok": True,
        "manifest": str(manifest_path),
        "assetDir": str(asset_dir),
        "packSource": str(pack_source),
        "assetCount": len(assets),
        "offsets": offsets,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", required=True, help="Path to final/manifest.json")
    parser.add_argument("--firmware-dir", default=str(default_firmware_dir()))
    parser.add_argument("--asset-dir")
    parser.add_argument("--pack-source")
    parser.add_argument("--lvgl-image")
    parser.add_argument("--python", default=sys.executable)
    return parser.parse_args()


def main() -> None:
    print(json.dumps(sync(parse_args()), indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main()
