# vcpkg port

Quick and dirty (and only windows) port for mysql/mysql-connector-cpp. Once vcpkg is installed, copy the mysqlconnectorcpp folder to the vcpkgroot/ports folder where the other ported libraries are.

Run the following command:

```
$ vcpkg install mysqlconnectorcpp --triplet x64-windows
```

to install.

The rest is automatized.
