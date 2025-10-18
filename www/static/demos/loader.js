let canvasIdCounter = 0;

async function startModule(canvas) {
  const moduleURL = canvas.dataset.module;
  if (!moduleURL) return null;

  const dir = moduleURL.substring(0, moduleURL.lastIndexOf('/') + 1);

  try {
    const moduleFactory = (await import(moduleURL)).default;
    const Module = await moduleFactory({
      canvas,
      __canvasSelector: `#${canvas.id}`,
      __canvasId: canvas.id,
      locateFile: (path) => dir + path,
      print: (msg) => console.log(`[${moduleURL}]`, msg),
      printErr: (msg) => console.error(`[${moduleURL}]`, msg),
    });
    const runMain = () => {
      if (Module.callMain) {
        Module.callMain([]);
      } else if (Module._main) {
        Module._main();
      }
    };
    try {
      runMain();
    } catch (err) {
      if (err === 'unwind' || (err && err.name === 'ExitStatus')) {
        return Module;
      }
      throw err;
    }
    return Module;
  } catch (err) {
    const message = err && err.message ? err.message : String(err);
    console.error('Failed to launch demo', moduleURL, err);
    canvas.insertAdjacentHTML('afterend', `<p class="warn">Demo failed to load: ${message}</p>`);
    throw err;
  }
}

function prepareCanvas(canvas) {
  const moduleURL = canvas.dataset.module;
  if (!moduleURL) return;

  const widthAttr = canvas.dataset.width || canvas.getAttribute('width');
  const heightAttr = canvas.dataset.height || canvas.getAttribute('height');
  if (widthAttr) canvas.width = Number.parseInt(widthAttr, 10);
  if (heightAttr) canvas.height = Number.parseInt(heightAttr, 10);

  if (!canvas.id) {
    canvas.id = `demo-canvas-${canvasIdCounter++}`;
  }
  if (!canvas.hasAttribute('tabindex')) {
    canvas.tabIndex = 0;
  }

  const poster = canvas.dataset.poster;
  const applyPoster = () => {
    if (!poster) return;
    canvas.style.backgroundImage = `url(${poster})`;
    canvas.style.backgroundSize = 'cover';
    canvas.style.backgroundPosition = 'center';
    canvas.style.backgroundRepeat = 'no-repeat';
    canvas.classList.add('with-poster');
  };
  const clearPoster = () => {
    if (!poster) return;
    canvas.style.backgroundImage = 'none';
    canvas.classList.remove('with-poster');
  };

  applyPoster();

  let modulePromise = null;
  let moduleExports = null;
  let setActive = null;
  let updateMouse = null;
  let started = false;
  const computeInitialVisibility = () => {
    const rect = canvas.getBoundingClientRect();
    const viewHeight = window.innerHeight || document.documentElement.clientHeight || 0;
    const viewWidth = window.innerWidth || document.documentElement.clientWidth || 0;
    return rect.bottom > 0 && rect.top < viewHeight && rect.right > 0 && rect.left < viewWidth;
  };
  let isVisible = !('IntersectionObserver' in window) ? true : computeInitialVisibility();
  let lastAppliedActive = null;
  const applyActiveState = () => {
    if (!setActive) return;
    const shouldBeActive = started && isVisible;
    if (lastAppliedActive === shouldBeActive) return;
    lastAppliedActive = shouldBeActive;
    try {
      setActive(shouldBeActive ? 1 : 0);
    } catch (err) {
      console.error('set_active state update failed', err);
    }
  };

  const ensureModule = async () => {
    if (!modulePromise) {
      canvas.classList.add('demo-activating');
      modulePromise = startModule(canvas)
        .then((Module) => {
          moduleExports = Module?.instance?.exports || Module?.asm || Module?.exports || Module;
          const candidate = moduleExports?.set_active || moduleExports?._set_active || Module?._set_active;
          const mouseCandidate = moduleExports?.update_mouse || moduleExports?._update_mouse || Module?._update_mouse;
          if (typeof candidate === 'function') setActive = (value) => candidate(value | 0);
          if (typeof mouseCandidate === 'function') updateMouse = (x, y, present) => mouseCandidate(x, y, present);
          if (Module?.cwrap) {
            if (!setActive) {
              try {
                const wrapped = Module.cwrap('set_active', null, ['number']);
                setActive = (value) => wrapped(value | 0);
              } catch (_) { setActive = null; }
            }
            if (!updateMouse) {
              try {
                const wrappedMouse = Module.cwrap('update_mouse', null, ['number', 'number', 'number']);
                updateMouse = (x, y, present) => wrappedMouse(x, y, present);
              } catch (_) { updateMouse = null; }
            }
          }
          applyActiveState();
          return Module;
        })
        .catch((err) => {
          modulePromise = null;
          applyPoster();
          throw err;
        })
        .finally(() => {
          canvas.classList.remove('demo-activating');
        });
    }
    return modulePromise;
  };

  const start = async () => {
    if (started) return;
    started = true;
    clearPoster();
    await ensureModule();
    applyActiveState();
    updateMouse?.(canvas.width * 0.5, canvas.height * 0.5, 0);
    try { canvas.focus({ preventScroll: true }); } catch (_) {}
  };

  const handlePointerMove = (ev) => {
    if (!started || !updateMouse || !isVisible) return;
    const rect = canvas.getBoundingClientRect();
    const scaleX = canvas.width / rect.width;
    const scaleY = canvas.height / rect.height;
    const x = (ev.clientX - rect.left) * scaleX;
    const y = (ev.clientY - rect.top) * scaleY;
    updateMouse(x, y, 1);
  };

  const handlePointerLeave = () => {
    if (!started) return;
    updateMouse?.(0, 0, 0);
  };

  canvas.addEventListener('pointerdown', start);
  canvas.addEventListener('keydown', (ev) => {
    if (!started && (ev.key === 'Enter' || ev.key === ' ')) {
      ev.preventDefault();
      start();
    }
  });
  canvas.addEventListener('pointermove', handlePointerMove, { passive: true });
  canvas.addEventListener('pointerleave', handlePointerLeave, { passive: true });
  canvas.addEventListener('blur', handlePointerLeave);

  if ('IntersectionObserver' in window) {
    const observer = new IntersectionObserver((entries) => {
      for (const entry of entries) {
        if (entry.target !== canvas) continue;
        const nowVisible = entry.isIntersecting && entry.intersectionRatio >= 0.1;
        if (nowVisible === isVisible) continue;
        isVisible = nowVisible;
        applyActiveState();
      }
    }, { threshold: [0, 0.1, 0.25, 0.5, 0.75, 1] });
    observer.observe(canvas);
  }
}

document.querySelectorAll('canvas[data-module]').forEach(prepareCanvas);
