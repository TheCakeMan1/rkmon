obj-m := rkmon.o

rkmon-objs := \
    rkmon_dev.o \
    src/cpu_update.o \
    src/freq_update.o \
    src/mem_update.o \
    src/network_update.o

EXTRA_CFLAGS += -I$(PWD)/include

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	@echo "[1] Installing module into kernel..."
	sudo $(MAKE) -C $(KDIR) M=$(PWD) modules_install
	sudo depmod -a

	@echo "[2] Loading module..."
	sudo modprobe rkmon || true

	@echo "[3] Creating /dev/rkmon..."
	@if ! grep -q rkmon /proc/devices; then \
		echo "ERROR: module not loaded — no major found"; exit 1; \
	fi
	@MAJOR=$$(grep rkmon /proc/devices | awk '{print $$1}'); \
	sudo rm -f /dev/rkmon; \
	sudo mknod /dev/rkmon c $$MAJOR 0; \
	sudo chmod 666 /dev/rkmon; \
	echo "/dev/rkmon created (major=$$MAJOR)"

uninstall:
	@echo "[1] Unloading module..."
	sudo modprobe -r rkmon || true

	@echo "[2] Removing module file..."
	sudo rm -f /lib/modules/$(shell uname -r)/extra/rkmon.ko
	sudo depmod -a

	@echo "[3] Removing /dev/rkmon..."
	sudo rm -f /dev/rkmon

	@echo "[DONE]"