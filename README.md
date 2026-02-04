# Exchange Disruptor

Projet C++20 base sur CMake pour experimenter des composants autour d'un moteur d'`exchange/trading`.

## Prerequis

- CMake >= 3.20
- Compilateur C++20 (AppleClang, Clang, GCC ou MSVC)
- `make` (ou adaptation des presets selon ton generateur)

## Build rapide (Debug)

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Lancer le binaire principal :

```bash
./build/debug-make/exchange_disruptor
```

## Build Release

```bash
cmake --preset release
cmake --build --preset release
ctest --preset release
```

## Activer "warnings as errors"

```bash
cmake --preset debug -DEXCHANGE_DISRUPTOR_WARNINGS_AS_ERRORS=ON
cmake --build --preset debug
```

## Structure

```text
.
├── CMakeLists.txt
├── CMakePresets.json
├── cmake/
│   └── Warnings.cmake
├── include/
├── src/
│   └── main.cpp
└── tests/
    ├── CMakeLists.txt
    └── test_main.cpp
```

## Commandes utiles

- Reconfigurer proprement un preset :
  ```bash
  rm -rf build/debug-make && cmake --preset debug
  ```
- Lister les tests :
  ```bash
  ctest --preset debug -N
  ```

## Prochaines evolutions conseillees

- Ajouter une vraie librairie metier (`src/lib.cpp`, headers dans `include/`).
- Integrer un framework de test (Catch2 ou GoogleTest).
- Ajouter un preset `asan`/`ubsan` pour durcir le debug.
