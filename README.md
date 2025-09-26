# Minimal WebGL Demo Site

This directory hosts a suckless-style static site that bundles a few C/OpenGL ES demos, compiled to WebAssembly with Emscripten. A tiny shared runtime spins up the WebGL2 canvas and calls into each demo.

## Layout

```
site/
├─ Makefile                     # builds HTML and compiles each demo + runtime to JS/WASM
├─ tpl/                         # shared HTML fragments (header/footer)
├─ src/                         # C sources for the demos
│  ├─ tri.c                     # rotating triangle with per-vertex colour
│  ├─ plasma.c                  # GPU plasma shader
│  ├─ mandelbrot.c              # Mandelbrot explorer with key controls
│  └─ boids.c                   # simple flocking simulation
│  ├─ runtime_webgl.c           # shared WebGL loop / platform bridge
│  └─ demo_app.h                # tiny interface each demo implements
└─ public/
   ├─ index.html.m4             # entry page template (rendered via m4)
   ├─ style.css                 # single stylesheet for the whole site
   ├─ demos/
   │  ├─ loader.js              # boots each compiled module into its canvas
   │  └─ <demo>/<demo>.js/.wasm # emitted by emcc (ES module factory + WASM)
   ├─ snippets/                 # generated HTML snippets with escaped C source
   └─ index.html                # generated output (do not edit directly)
```

## Building

1. Activate an Emscripten toolchain (`source /path/to/emsdk_env.sh`).
2. From this `site/` directory run:

   ```sh
   make
   ```

This runs `m4`, generates the code snippets, and compiles each demo (`src/<name>.c`) with `src/runtime_webgl.c`. Every target produces `public/demos/<name>/<name>.js` plus the matching `<name>.wasm`.

3. Serve `public/` with any static server that sends `application/wasm` for `.wasm`, for example:

   ```sh
   python3 -m http.server -d public 8000
   ```

   Then open <http://localhost:8000/> in a browser.

## Extending

- Drop a new C file into `src/`, implement the `demo_app_*` hooks, and add its basename to `DEMOS` in the `Makefile`. The build will emit `public/demos/<name>/<name>.js/.wasm`.
- Add a `<section>` with a `<canvas data-module="/demos/<name>/<name>.js">` block to `public/index.html.m4` so the loader picks it up.
- Keep the templates readable for no-JS visitors by including `<noscript>` fallbacks that point to the source.

## Cleaning

```sh
make clean
```

This removes the generated `public/index.html` and all compiled demo outputs so you can rebuild from scratch.
