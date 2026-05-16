# modules/08_templates_basics — 模板基础 / Template Basics
#
# 演示：constexpr/consteval/constinit、模板特化、重载决议、
# constexpr if、两阶段查找、concepts、万能引用与完美转发。

mcpp_add_demo(NAME constexpr_basics           SOURCES demos/constexpr_basics.cpp)
mcpp_add_demo(NAME consteval_constinit        SOURCES demos/consteval_constinit.cpp)
mcpp_add_demo(NAME template_specialization    SOURCES demos/template_specialization.cpp)
mcpp_add_demo(NAME overload_resolution        SOURCES demos/overload_resolution.cpp)
mcpp_add_demo(NAME constexpr_if               SOURCES demos/constexpr_if.cpp)
mcpp_add_demo(NAME two_phase_lookup           SOURCES demos/two_phase_lookup.cpp)
mcpp_add_demo(NAME typename_template          SOURCES demos/typename_template.cpp)
mcpp_add_demo(NAME concepts_basics            SOURCES demos/concepts_basics.cpp)
mcpp_add_demo(NAME concept_subsumption        SOURCES demos/concept_subsumption.cpp)
mcpp_add_demo(NAME universal_ref              SOURCES demos/universal_ref.cpp)

mcpp_add_test(NAME test_constexpr             SOURCES tests/test_constexpr.cpp)
mcpp_add_test(NAME test_specialization        SOURCES tests/test_specialization.cpp)
mcpp_add_test(NAME test_constexpr_if          SOURCES tests/test_constexpr_if.cpp)
mcpp_add_test(NAME test_concepts              SOURCES tests/test_concepts.cpp)
mcpp_add_test(NAME test_concept_subsumption   SOURCES tests/test_concept_subsumption.cpp)
mcpp_add_test(NAME test_universal_ref         SOURCES tests/test_universal_ref.cpp)
mcpp_add_test(NAME test_ref_collapsing        SOURCES tests/test_ref_collapsing.cpp)
