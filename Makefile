SHELL := /bin/sh
EMCC ?= emcc
DEMOS := tri plasma mandelbrot boids
HTML := public/index.html
DEMO_JS := $(foreach d,$(DEMOS),public/demos/$(d)/$(d).js)
SNIPPETS := $(foreach d,$(DEMOS),public/snippets/$(d).html)
DEMOS_DIR   := public/demos
DEMOS_PAGE  := $(DEMOS_DIR)/index.html
DEMO_WASM   := $(foreach d,$(DEMOS),public/demos/$(d)/$(d).wasm)


EMCC_FLAGS := -O3 -s USE_WEBGL2=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 \
	-s MODULARIZE=1 -s EXPORT_ES6=1 -s INVOKE_RUN=0 -s EXIT_RUNTIME=0 \
	-s FORCE_FILESYSTEM=0 -s ALLOW_MEMORY_GROWTH=1 -s FULL_ES3=1 \
	-s EXPORTED_RUNTIME_METHODS='["stringToUTF8","lengthBytesUTF8","cwrap"]'

all: $(HTML) $(DEMO_JS) $(DEMOS_PAGE) 

public/index.html: public/index.html.m4 tpl/header.html tpl/footer.html $(SNIPPETS) | public
	m4 $< > $@

public:
	mkdir -p $@

public/demos:
	mkdir -p $@

public/snippets:
	mkdir -p $@

define BUILD_DEMO
public/demos/$(1)/$(1).js: src/$(1).c src/runtime_webgl.c src/demo_app.h | public/demos
	mkdir -p $$(@D)
	$(EMCC) src/runtime_webgl.c src/$(1).c $(EMCC_FLAGS) -Isrc -o $$@
endef
$(foreach d,$(DEMOS),$(eval $(call BUILD_DEMO,$(d))))

public/snippets/%.html: src/%.c | public/snippets
	python3 -c 'import html, pathlib, sys; src = pathlib.Path(sys.argv[1]).read_text(); esc = html.escape(src); pathlib.Path(sys.argv[2]).write_text("<pre><code class=\"language-c\">" + esc + "</code></pre>\n")' "$<" "$@"

$(DEMOS_PAGE): $(DEMO_JS) $(DEMO_WASM) | $(DEMOS_DIR)
	{ \
	  echo '<!doctype html>'; \
	  echo '<meta charset="utf-8">'; \
	  echo '<title>Demos</title>'; \
	  echo '<pre>'; \
	  for d in $(DEMOS); do \
	    echo "$$d/"; \
	    echo "  - <a href=\"./$$d/$$d.js\" download>$$d.js</a>"; \
	    echo "  - <a href=\"./$$d/$$d.wasm\" download>$$d.wasm</a>"; \
	    echo ""; \
	  done; \
	  echo '</pre>'; \
	} > $@

clean:
	rm -f $(HTML)
	rm -rf $(foreach d,$(DEMOS),public/demos/$(d))
	rm -rf public/snippets

.PHONY: all clean
