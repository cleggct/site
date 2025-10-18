# Minimal WebGL Demo Site

This directory hosts a static site that bundles a few C/OpenGL ES demos, compiled to WebAssembly with Emscripten. A tiny shared runtime spins up the WebGL2 canvas and calls into each demo. The page chrome is now generated with [Eleventy](https://www.11ty.dev/).

## Layout

```
site/
├─ Makefile                  # runs Eleventy and compiles each demo with Emscripten
├─ eleventy.config.js        # Eleventy configuration (shortcodes + passthrough assets)
├─ package.json              # Eleventy scripts/dev dependencies
├─ src/                      # C sources for the demos
│  ├─ tri.c
│  ├─ plasma.c
│  ├─ mandelbrot.c
│  ├─ boids.c
│  ├─ runtime_webgl.c        # shared WebGL loop / platform bridge
│  └─ demo_app.h             # tiny interface each demo implements
├─ www/                      # Eleventy input
│  ├─ index.njk              # homepage template
│  ├─ demos/index.njk        # download listing for compiled demos
│  ├─ _includes/base.njk     # shared layout (header/footer)
│  ├─ _data/demos.js         # metadata for each demo shown on the homepage
│  └─ static/                # passthrough assets (CSS, images, loader.js)
└─ public/                   # generated output (do not edit directly)
```

## Building

1. Install the Eleventy toolchain (once per clone):

   ```sh
   npm install
   ```

2. Activate an Emscripten toolchain (`source /path/to/emsdk_env.sh`).

3. From this `site/` directory run:

   ```sh
   make
   ```

   The `Makefile` runs Eleventy (`npx @11ty/eleventy`) to emit `public/index.html` and static assets, then compiles each demo (`src/<name>.c`) together with `src/runtime_webgl.c`. Every target produces `public/demos/<name>/<name>.js` plus the matching `<name>.wasm`.

4. Serve `public/` with any static server that sends `application/wasm` for `.wasm`, for example:

   ```sh
   python3 -m http.server -d public 8000
   ```

   Then open <http://localhost:8000/> in a browser.

For local development you can run `npm run watch` in one terminal (Eleventy dev server) and `make DEMOS=<subset>` in another to rebuild the WebAssembly outputs as needed.

## Extending

- Drop a new C file into `src/`, implement the `demo_app_*` hooks, and add its basename to `DEMOS` in the `Makefile`. The build will emit `public/demos/<name>/<name>.js/.wasm`.
- Add its metadata to `www/_data/demos.js`. The homepage template iterates over that list and uses the `codeSnippet` shortcode to pull source straight from `src/<name>.c`.
- If you need bespoke markup, extend `www/index.njk` (which uses the shared layout in `www/_includes/base.njk`).

## Cleaning

```sh
make clean
```

This removes the generated `public/` directory so you can rebuild from scratch.

## Note About AI Usage in this Repo

Generative AI (gpt-5/codex) was used in the generation of this README and in troubleshooting compilation errors with the demos. The code (shaders, templates, webgl stuff) was written by me with a hefty dose of help from the emscripten documentation (<https://emscripten.org/docs/>).
