#!/usr/bin/env python3
"""Genera/actualiza los archivos binarios PERFILES.DAT y USUARIOS.DAT.

El programa principal NO necesita este script: lee y escribe directamente los
archivos configurados en USER_FILE / PERFIL_FILE (por defecto USUARIOS.TXT y
PERFILES.TXT), que ya estan en formato binario de structs de tamano fijo.
Este script sirve para:
  - convertir archivos de texto antiguos (lineas con ';') al formato binario, o
  - copiar/validar archivos que ya son binarios.

"""
import os
import struct
import sys

# ----------------------------------------------------------------------------
# DEFINICIÓN DE MOLDES BINARIOS (STRUCTS)
# Aquí se define exactamente cómo C++ leerá la memoria. "<" significa Little Endian.
# "i" = entero (4 bytes), "s" = cadena de texto (chars), "x" = byte de relleno.
# ----------------------------------------------------------------------------
USUARIO_FMT = "<i50s30s30s30s"   # 144 bytes  (struct Usuario, include/usuario.h)
PERFIL_FMT = "<30s2xi20i"        # 116 bytes  (struct Perfil,  include/perfil.h)
TAM_USUARIO = struct.calcsize(USUARIO_FMT)
TAM_PERFIL = struct.calcsize(PERFIL_FMT)
assert TAM_USUARIO == 144
assert TAM_PERFIL == 116
MAX_OPCIONES = 20


def es_binario(ruta):
    with open(ruta, "rb") as f:
        return b"\0" in f.read(4096)


def leer_perfiles_binarios(ruta):
    datos = open(ruta, "rb").read()
    if len(datos) % TAM_PERFIL != 0:
        raise ValueError(f"{ruta}: {len(datos)} bytes no es multiplo de {TAM_PERFIL} (struct Perfil)")
    perfiles = []
    for i in range(0, len(datos), TAM_PERFIL):
        nombre, cantidad, *ops = struct.unpack(PERFIL_FMT, datos[i:i + TAM_PERFIL])
        perfiles.append((nombre.split(b"\0")[0].decode("utf-8", "replace"), list(ops[:cantidad])))
    return perfiles


def leer_perfiles_texto(ruta):
    perfiles = []
    with open(ruta, encoding="utf-8", errors="replace") as f:
        for linea in f:
            linea = linea.strip()
            if not linea or ";" not in linea:
                continue
            nombre, ops = linea.split(";", 1)
            opciones = [int(x) for x in ops.split(",") if x.strip().isdigit()][:MAX_OPCIONES]
            perfiles.append((nombre, opciones))
    return perfiles


def escribir_perfiles(perfiles, destino):
    with open(destino, "wb") as f:
        for nombre, opciones in perfiles:
            relleno = opciones + [0] * (MAX_OPCIONES - len(opciones))
            f.write(struct.pack(PERFIL_FMT, nombre.encode("utf-8")[:29], len(opciones), *relleno))
    print(f"{destino}: {len(perfiles)} perfil(es) escritos ({TAM_PERFIL} bytes c/u)")


def leer_usuarios_binarios(ruta):
    datos = open(ruta, "rb").read()
    if len(datos) % TAM_USUARIO != 0:
        raise ValueError(f"{ruta}: {len(datos)} bytes no es multiplo de {TAM_USUARIO} (struct Usuario)")
    usuarios = []
    for i in range(0, len(datos), TAM_USUARIO):
        uid, *campos = struct.unpack(USUARIO_FMT, datos[i:i + TAM_USUARIO])
        usuarios.append((uid, *[c.split(b"\0")[0].decode("latin-1") for c in campos]))
    return usuarios


def leer_usuarios_texto(ruta, perfiles_validos):
    usuarios = []
    with open(ruta, encoding="utf-8", errors="replace") as f:
        for n, linea in enumerate(f, 1):
            partes = linea.rstrip("\r\n").split(";")
            if len(partes) != 5:
                print(f"  omitida linea {n}: formato invalido")
                continue
            id_str, nombre, username, password, perfil = partes
            if not id_str.isdigit() or int(id_str) <= 0 or not password or perfil not in perfiles_validos:
                print(f"  omitida linea {n}: datos invalidos")
                continue
            usuarios.append((int(id_str), nombre, username, password, perfil))
    return usuarios


def escribir_usuarios(usuarios, destino):
    with open(destino, "wb") as f:
        for uid, nombre, username, password, perfil in usuarios:
            f.write(struct.pack(USUARIO_FMT, uid,
                                nombre.encode("latin-1", "replace")[:49],
                                username.encode("latin-1", "replace")[:29],
                                password.encode("latin-1", "replace")[:29],
                                perfil.encode("latin-1", "replace")[:29]))
    print(f"{destino}: {len(usuarios)} usuario(s) escritos ({TAM_USUARIO} bytes c/u)")


if __name__ == "__main__":
    if not os.path.exists("PERFILES.TXT"):
        sys.exit("No existe PERFILES.TXT (ejecute el script desde la carpeta del proyecto)")
    perfiles = (leer_perfiles_binarios if es_binario("PERFILES.TXT") else leer_perfiles_texto)("PERFILES.TXT")
    escribir_perfiles(perfiles, "PERFILES.DAT")

    if os.path.exists("USUARIOS.TXT"):
        if es_binario("USUARIOS.TXT"):
            usuarios = leer_usuarios_binarios("USUARIOS.TXT")
        else:
            usuarios = leer_usuarios_texto("USUARIOS.TXT", {p[0] for p in perfiles})
        escribir_usuarios(usuarios, "USUARIOS.DAT")
