SHELL := /bin/sh
EMCC ?= emcc
DEMOS := tri plasma mandelbrot boids
HTML := public/index.html
DEMO_JS := $(foreach d,$(DEMOS),public/demos/$(d)/$(d).js)
SNIPPETS := $(foreach d,$(DEMOS),public/snippets/$(d).html)

EMCC_FLAGS := -O3 -s USE_WEBGL2=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 \
	-s MODULARIZE=1 -s EXPORT_ES6=1 -s INVOKE_RUN=0 -s EXIT_RUNTIME=0 \
	-s FORCE_FILESYSTEM=0 -s ALLOW_MEMORY_GROWTH=1 -s FULL_ES3=1 \
	-s EXPORTED_RUNTIME_METHODS='["stringToUTF8","lengthBytesUTF8","cwrap"]'

all: $(HTML) $(DEMO_JS)

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

clean:
	rm -f $(HTML)
	rm -rf $(foreach d,$(DEMOS),public/demos/$(d))
	rm -rf public/snippets

.PHONY: all clean
