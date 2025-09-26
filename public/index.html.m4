include(`tpl/header.html')
<section class="intro">
  <img class="profile" src="/profile.png" alt="Profile photo of Christa Clegg">
  <div class="intro-copy">
    <h1>Christa Clegg</h1>
    <p>Junior developer in Oulu shipping tidy web apps and LLM-flavoured data tooling.</p>
    <p class="contact">cleggct (at) gmail (dot) com · <a href="https://github.com/cleggct" rel="noopener">GitHub</a> · <a href="https://www.linkedin.com/in/christa-clegg/" rel="noopener">LinkedIn</a></p>
  </div>
</section>

<p class="overview">C/OpenGL ES experiments compiled to WebAssembly. Click or tap a canvas to launch it; it pauses automatically when scrolled off screen.</p>

<section class="demo">
  <h2>Rotating Triangle</h2>
  <p>Classic GL triangle with a rotation and a brightening color pulse.</p>
  <noscript>
    <p>Enable JavaScript to run the WebGL demo. The source lives in <code>src/tri.c</code>.</p>
  </noscript>
  <canvas data-module="/demos/tri/tri.js" data-width="640" data-height="360" data-poster="/posters/tri.png"></canvas>
  <details class="source">
    <summary>Source: <code>src/tri.c</code></summary>
    include(`public/snippets/tri.html')
  </details>
</section>

<section class="demo">
  <h2>Plasma Shader</h2>
  <p>Full-screen plasma driven by layered sine waves and a little swirl math.</p>
  <noscript>
    <p>Enable JavaScript to run the WebGL demo. The source lives in <code>src/plasma.c</code>.</p>
  </noscript>
  <canvas data-module="/demos/plasma/plasma.js" data-width="640" data-height="360" data-poster="/posters/plasma.png"></canvas>
  <details class="source">
    <summary>Source: <code>src/plasma.c</code></summary>
    include(`public/snippets/plasma.html')
  </details>
</section>

<section class="demo">
  <h2>Mandelbrot Explorer</h2>
  <p>Keyboard-driven Mandelbrot (arrows to pan, Z/X to zoom) rendered in plain GLSL.</p>
  <noscript>
    <p>Enable JavaScript to run the WebGL demo. The source lives in <code>src/mandelbrot.c</code>.</p>
  </noscript>
  <canvas data-module="/demos/mandelbrot/mandelbrot.js" data-width="640" data-height="360" data-poster="/posters/mandelbrot.png"></canvas>
  <details class="source">
    <summary>Source: <code>src/mandelbrot.c</code></summary>
    include(`public/snippets/mandelbrot.html')
  </details>
</section>

<section class="demo">
  <h2>Boids</h2>
  <p>Flocking simulation that follows your pointer; once it leaves the canvas a velocity damping term settles the flock.</p>
  <noscript>
    <p>Enable JavaScript to run this demo. The source lives in <code>src/boids.c</code>.</p>
  </noscript>
  <canvas data-module="/demos/boids/boids.js" data-width="640" data-height="360" data-poster="/posters/boids.png"></canvas>
  <details class="source">
    <summary>Source: <code>src/boids.c</code></summary>
    include(`public/snippets/boids.html')
  </details>
</section>

<script type="module" src="/demos/loader.js"></script>
include(`tpl/footer.html')
