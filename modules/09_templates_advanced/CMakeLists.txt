# modules/09_templates_advanced — 模板进阶 / Advanced Templates
#
# 演示：模板模板形参、NTTP、CTAD、友元、惰性实例化、
# 可变参数模板、折叠表达式、SFINAE、CRTP、类型擦除。

mcpp_add_demo(NAME template_template_param SOURCES demos/template_template_param.cpp)
mcpp_add_demo(NAME nttp_demo               SOURCES demos/nttp_demo.cpp)
mcpp_add_demo(NAME type_deduction         SOURCES demos/type_deduction.cpp)
mcpp_add_demo(NAME ctad_demo              SOURCES demos/ctad_demo.cpp)
mcpp_add_demo(NAME friends_in_templates   SOURCES demos/friends_in_templates.cpp)
mcpp_add_demo(NAME lazy_instantiation     SOURCES demos/lazy_instantiation.cpp)
mcpp_add_demo(NAME variadic_templates     SOURCES demos/variadic_templates.cpp)
mcpp_add_demo(NAME fold_expressions       SOURCES demos/fold_expressions.cpp)
mcpp_add_demo(NAME sfinae_demo            SOURCES demos/sfinae_demo.cpp)
mcpp_add_demo(NAME crtp_type_erasure      SOURCES demos/crtp_type_erasure.cpp)

mcpp_add_test(NAME test_nttp          SOURCES tests/test_nttp.cpp)
mcpp_add_test(NAME test_ctad          SOURCES tests/test_ctad.cpp)
mcpp_add_test(NAME test_variadic      SOURCES tests/test_variadic.cpp)
mcpp_add_test(NAME test_fold          SOURCES tests/test_fold.cpp)
mcpp_add_test(NAME test_sfinae        SOURCES tests/test_sfinae.cpp)
mcpp_add_test(NAME test_crtp          SOURCES tests/test_crtp.cpp)
mcpp_add_test(NAME test_type_erasure  SOURCES tests/test_type_erasure.cpp)
mcpp_add_test(NAME test_template_template_param SOURCES tests/test_template_template_param.cpp)
mcpp_add_test(NAME test_friends SOURCES tests/test_friends.cpp)
mcpp_add_test(NAME test_lazy_instantiation SOURCES tests/test_lazy_instantiation.cpp)
mcpp_add_test(NAME test_type_deduction SOURCES tests/test_type_deduction.cpp)
