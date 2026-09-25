CXX      ?= g++
OUT      := out
SRC      := src

# 默认走低内存参数，避免在手机上编译时把系统拖垮。
# 想要优化版本：make CXXFLAGS="-std=c++17 -O2 -Wall -Wextra -Isrc"
CXXFLAGS ?= -std=c++17 -O0 -Wall -Isrc \
            -fno-var-tracking -fno-var-tracking-assignments -fno-inline \
            --param ggc-min-expand=10 --param ggc-min-heapsize=8192

all: interp

interp: $(OUT)/interp

$(OUT)/interp: $(SRC)/interp.cpp $(SRC)/frontend.hpp $(SRC)/token.hpp $(SRC)/keyword.hpp
	@mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC)/interp.cpp
	@echo "  [OK] $(OUT)/interp"

test: interp
	@for f in examples/*.re; do \
		printf '%-28s' "$$f"; \
		if $(OUT)/interp "$$f" >/dev/null 2>&1; then echo "OK"; else echo "FAIL"; fi; \
	done

clean:
	rm -rf $(OUT)
	@echo "  [OK] 已清理"

.PHONY: all interp test clean