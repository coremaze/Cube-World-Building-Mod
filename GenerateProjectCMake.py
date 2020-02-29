import os

path = os.path.dirname(os.path.realpath(__file__))
_, folderName = os.path.split(path)

lines = [
'cmake_minimum_required (VERSION 3.8)',
'project(project_%s)' % folderName,

'add_subdirectory(CWSDK)',

'add_library (%s SHARED "main.cpp")' % folderName,

'target_link_libraries (%s LINK_PUBLIC CWSDK)' % folderName,

    ]

with open(os.path.join(path, 'CMakeLists.txt'), 'w') as f:
    f.write('\n'.join(lines))
