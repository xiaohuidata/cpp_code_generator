# config/Makefile.am
configdir = $(datarootdir)/cpp_code_generator/config

config_DATA = \
    templates/singleton.json \
    templates/factory.json \
    templates/observer.json \
    examples/basic_project.json \
    examples/advanced_project.json \
    examples/test_project.json

# 配置子目录
include templates/Makefile.am
include examples/Makefile.am