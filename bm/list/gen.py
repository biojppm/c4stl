import os
import sys
import re
from collections import OrderedDict as odict
import jinja2


value_types = ["int", "NumBytes<64>", "NumBytes<4096>"]
size_types = ["uint64_t", "int64_t", "uint32_t", "int32_t"]
storage_types = ["fixed", "raw", "small", "paged", "paged_rt"]
topics = odict([
    ("list", odict([
        ("container_types", odict([
            ("std::list",
             "std::list< {T} >"),

            ("flat_list__{S}",
             "flat_list__{S}< {T}, {I} >"),
            ("split_list__{S}",
             "split_list__{S}< {T}, {I} >"),
            ("flat_fwd_list__{S}",
             "flat_fwd_list__{S}< {T}, {I} >"),
            ("split_fwd_list__{S}",
             "split_fwd_list__{S}< {T}, {I} >"),
        ])),
        ("benchmarks", odict([
             ("push_back", "push_back.cpp.tpl"),
             ("push_back_with_reserve", "push_back_with_reserve.cpp.tpl"),
        ])),
    ])),
])


class BmCreator:

    cmake_code = """\
# Generated automatically. Do not edit.
"""

    def __init__(self, topic, name, tpl_file):
        self.topic = topic
        self.name = name
        self.tpl_file = tpl_file
        with open(tpl_file) as f:
            self.tpl_str = f.read()
            self.tpl = jinja2.Template(self.tpl_str)
        self.count = 0
        BmCreator.cmake_code += """\

# benchmark cases for """ + name + """
"""

    def create_one(self, container, T, I, S):
        self.count += 1
        d = {}
        d['T'] = T
        if T and I and S:
            type_name = container.format(T=T, I=I, S=S)
            cpp_name = re.sub(r'.cpp.tpl', r'', self.tpl_file) + '__{S}__{T}__{I}'
            cpp_name = cpp_name.format(T=T, I=I, S=S)
            d['I'] = I
            d['S'] = S
        elif T and (not I) and (not S):
            type_name = container.format(T=T)
            cpp_name = re.sub(r'.cpp.tpl', r'', self.tpl_file) + '__{T}'
            cpp_name = cpp_name.format(T=T)
        identifier = re.sub(r'[<>, ]', r'_', type_name)
        identifier = re.sub(r'::', r'_', identifier)
        identifier = re.sub(r'_*$', r'', identifier)
        cpp_name = self.name + '/' + identifier + '.cpp'
        d['type_name'] = type_name
        cm = "c4stl_add_bm({topic}-{name}-{identifier}    {cpp_name})\n"
        cm = cm.format(topic=self.topic,
                       name=self.name,
                       identifier=identifier,
                       cpp_name=cpp_name)
        BmCreator.cmake_code += cm
        code = self.tpl.render(d)
        if not os.path.exists(self.name):
            os.makedirs(self.name)
        with open(cpp_name, "w") as f:
            f.write(code)


def create_all():
    cmake_code = """\
# Generated automatically. Do not edit.
"""
    for topic_name, topic_dict in topics.items():
        benchmarks = topic_dict["benchmarks"]
        container_types = topic_dict["container_types"]
        for bm_name, bm_tpl in benchmarks.items():
            out = BmCreator(topic_name, bm_name, bm_tpl)
            for _, ct in container_types.items():
                has_T = ("{T}" in ct)
                has_I = ("{I}" in ct)
                has_S = ("{S}" in ct)
                if has_T and has_I and has_S:
                    for T in value_types:
                        for I in size_types:
                            for S in storage_types:
                                out.create_one(ct, T, I, S)
                elif has_T and (not has_I) and (not has_S):
                    for T in value_types:
                        out.create_one(ct, T, None, None)
                else:
                    raise Exception("???: " + ct)
            print("{}: generated {} cases".format(out.name, out.count))
        with open("CMakeLists.txt", "w") as f:
            f.write(BmCreator.cmake_code)


if __name__ == "__main__":
    create_all()
