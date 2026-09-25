CXX      ?= g++
OUT      := out
SRC      := src

UNAME_S  := $(shell uname -s | tr 'A-Z' 'a-z')
UNAME_M  := $(shell uname -m)
BIN      := interp_$(UNAME_S)_$(UNAME_M)

CXXFLAGS ?= -std=c++17 -O0 -Wall -Isrc \
            -fno-var-tracking -fno-var-tracking-assignments -fno-inline \
            --param ggc-min-expand=10 --param ggc-min-heapsize=8192

all: interp

interp: $(OUT)/$(BIN)

$(OUT)/$(BIN): $(SRC)/interp.cpp $(SRC)/frontend.hpp $(SRC)/token.hpp $(SRC)/keyword.hpp
	@mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC)/interp.cpp
	@echo "  [OK] $(OUT)/$(BIN)"

test: interp
	@for f in examples/*.re; do \
		printf '%-28s' "$$f"; \
		if $(OUT)/$(BIN) "$$f" >/dev/null 2>&1; then echo "OK"; else echo "FAIL"; fi; \
	done

release:
	@mkdir -p $(OUT)
	$(CXX) -std=c++17 -O2 -Wall -Wextra -Isrc -o $(OUT)/$(BIN) $(SRC)/interp.cpp
	@strip $(OUT)/$(BIN) 2>/dev/null || true
	@echo "  [OK] $(OUT)/$(BIN)"

clean:
	rm -rf $(OUT)
	@echo "  [OK] 已清理"

.PHONY: all interp test release clean