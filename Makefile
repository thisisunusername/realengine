CXX      ?= g++
OUT      := out
SRC      := src
UNAME_S  := $(shell uname -s 2>/dev/null | tr 'A-Z' 'a-z')
UNAME_M  := $(shell uname -m 2>/dev/null)

# Windows 探测：native cmd.exe 下 uname 不存在 → UNAME_S 为空
ifeq ($(UNAME_S),)
  OS      := windows
  EXE     := .exe
  UNAME_M := x86_64
else ifneq (,$(findstring mingw,$(UNAME_S)))
  OS      := windows
  EXE     := .exe
else ifneq (,$(findstring msys,$(UNAME_S)))
  OS      := windows
  EXE     := .exe
else ifneq (,$(findstring cygwin,$(UNAME_S)))
  OS      := windows
  EXE     := .exe
else
  OS      := $(UNAME_S)
  EXE     :=
endif

BIN      := interp_$(OS)_$(UNAME_M)$(EXE)
CXXFLAGS ?= -std=c++17 -O2 -Wall -Isrc

all: interp
interp: $(OUT)/$(BIN)
$(OUT)/$(BIN): $(SRC)/interp.cpp $(SRC)/frontend.hpp $(SRC)/token.hpp $(SRC)/keyword.hpp
	@mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC)/interp.cpp
	@echo "  [OK] $(OUT)/$(BIN)"
test: interp
	@for f in examples/*.re; do \
		printf '%-28s' "$$f"; \
		if $(OUT)/$(BIN) "$$f" </dev/null >/dev/null 2>&1; then echo "OK"; else echo "FAIL"; fi; \
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