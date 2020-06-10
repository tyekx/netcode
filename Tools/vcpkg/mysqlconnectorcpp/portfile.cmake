include(vcpkg_common_functions)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO mysql/mysql-connector-cpp
    REF 8.0.20
    SHA512 f2ed52d4d7cb8735cda6c3e684c9489016895f11ace5849affc8d6792c9bf384946edca37cccf9b2524644e2f6dc893e7156eea75ea773543fb53f1041a11d46
    HEAD_REF master
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    DISABLE_PARALLEL_CONFIGURE
    PREFER_NMAKE
    OPTIONS
        -DWITH_SSL=${CURRENT_INSTALLED_DIR}
)

vcpkg_install_cmake()

vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

if(EXISTS ${CURRENT_PACKAGES_DIR}/INFO_BIN)
    file(REMOVE "${CURRENT_PACKAGES_DIR}/INFO_BIN")
endif()

if(EXISTS "${CURRENT_PACKAGES_DIR}/INFO_SRC")
    file(REMOVE "${CURRENT_PACKAGES_DIR}/INFO_SRC")
endif()

if(EXISTS ${CURRENT_PACKAGES_DIR}/debug/INFO_BIN)
    file(REMOVE "${CURRENT_PACKAGES_DIR}/debug/INFO_BIN")
endif()

if(EXISTS "${CURRENT_PACKAGES_DIR}/debug/INFO_SRC")
    file(REMOVE "${CURRENT_PACKAGES_DIR}/debug/INFO_SRC")
endif()

file(COPY "${CURRENT_PACKAGES_DIR}/lib/mysqlcppconn8-2-vs14.dll" DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
file(REMOVE "${CURRENT_PACKAGES_DIR}/lib/mysqlcppconn8-2-vs14.dll")

file(COPY "${CURRENT_PACKAGES_DIR}/lib/vs14/mysqlcppconn8.lib" DESTINATION "${CURRENT_PACKAGES_DIR}/lib")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/lib/vs14")

file(COPY "${CURRENT_PACKAGES_DIR}/debug/lib/debug/mysqlcppconn8-2-vs14.dll" DESTINATION "${CURRENT_PACKAGES_DIR}/debug/bin")
file(COPY "${CURRENT_PACKAGES_DIR}/debug/lib/debug/mysqlcppconn8-2-vs14.pdb" DESTINATION "${CURRENT_PACKAGES_DIR}/debug/bin")
file(COPY "${CURRENT_PACKAGES_DIR}/debug/lib/vs14/debug/mysqlcppconn8.lib" DESTINATION "${CURRENT_PACKAGES_DIR}/debug/lib")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/lib/vs14")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/lib/debug")

file(INSTALL ${SOURCE_PATH}/LICENSE.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
