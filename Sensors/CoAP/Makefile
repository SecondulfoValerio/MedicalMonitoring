CONTIKI_PROJECT = coap-actuator
all: $(CONTIKI_PROJECT)

MODULES_REL += /home/iot_ubuntu_intel/contiki-ng/examples/emlearn 
INC += /home/iot_ubuntu_intel/contiki-ng/examples/emlearn
TARGET_LIBFILES += -lm
CFLAGS += -DPROJECT_CONF_PATH=\"project-conf.h\"
CONTIKI = ../..

#COAP
MODULES_REL += ./resources
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap
include $(CONTIKI)/Makefile.include


