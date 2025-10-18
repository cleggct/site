SHELL := /bin/sh
EMCC ?= emcc

DEMOS := tri plasma mandelbrot boids
BUILD_DIR := public
ELEVENTY_CMD ?= npx @11ty/eleventy

DEMO_JS := $(foreach d,$(DEMOS),$(BUILD_DIR)/demos/$(d)/$(d).js)
DEMO_WASM := $(foreach d,$(DEMOS),$(BUILD_DIR)/demos/$(d)/$(d).wasm)
ELEVENTY_STAMP := $(BUILD_DIR)/.eleventy-built
ELEVENTY_SOURCES := $(shell find www -type f) eleventy.config.js package.json $(wildcard package-lock.json)

EMCC_FLAGS := -O3 -s USE_WEBGL2=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 \
	-s MODULARIZE=1 -s EXPORT_ES6=1 -s INVOKE_RUN=0 -s EXIT_RUNTIME=0 \
	-s FORCE_FILESYSTEM=0 -s ALLOW_MEMORY_GROWTH=1 -s FULL_ES3=1 \
	-s EXPORTED_RUNTIME_METHODS='["stringToUTF8","lengthBytesUTF8","cwrap"]'

all: $(ELEVENTY_STAMP) $(DEMO_JS)

$(ELEVENTY_STAMP): $(ELEVENTY_SOURCES)
	$(ELEVENTY_CMD)
	@touch $@

define BUILD_DEMO
$(BUILD_DIR)/demos/$(1)/$(1).js: src/$(1).c src/runtime_webgl.c src/demo_app.h $(ELEVENTY_STAMP)
	mkdir -p $$(@D)
	$(EMCC) src/runtime_webgl.c src/$(1).c $(EMCC_FLAGS) -Isrc -o $$@
endef
$(foreach d,$(DEMOS),$(eval $(call BUILD_DEMO,$(d))))

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
