
## Build

### Fetch Dependencies

```bash
python3 ./fetch_repos.py
```

### Tool Chains

[ESP-IDF v5.5.4](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/index.html)

On the current development machine, ESP-IDF v5.5.4 is installed at:

```bash
~/esp/esp-idf-v5.5.4
```

Open a new shell and enable it with:

```bash
get_idf
```

### Build

```bash
idf.py build
```

### Build ImageAvatar Skin

Create a local overlay. This file is git-ignored:

```bash
cat > sdkconfig.defaults.local <<'EOF'
# CONFIG_STACKCHAN_AVATAR_SKIN_DEFAULT is not set
CONFIG_STACKCHAN_AVATAR_SKIN_IMAGE=y
EOF
rm -f sdkconfig sdkconfig.old
idf.py fullclean
idf.py build
```

Remove `sdkconfig.defaults.local` and regenerate `sdkconfig` when returning to
the default official avatar skin.

### Host-side tests

The motion coordinate helpers can be tested without ESP-IDF hardware:

```bash
cmake -S tests -B build-host-tests
cmake --build build-host-tests
ctest --test-dir build-host-tests --output-on-failure
```

### Flash

```bash
idf.py flash
```
