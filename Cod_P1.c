/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: frnk
 *
 * Created on 15 de junio de 2019, 03:59 PM
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    char Status;
    char Type;
    char Fit;
    int Start;
    int Size;
    char Name[16];
} Particion;

typedef struct {
    int Size;
    char FCreacion[20];
    int ID;
    char Fit;
    Particion Particiones[4];
} MBR;

typedef struct {
    char Status;
    char Fit;
    int Start;
    int Size;
    int Next;
    char Name[16];
} EBR;

typedef struct {
    char Status;
    char Type;
    char Fit;
    int Start;
    int Size;
    char ID[5];
    char Name[16];
} Particion_Montada;

typedef struct {
    Particion_Montada Particiones[10];
    char Path[300];
    int letra;
} Disco_Montado;

typedef struct { //BM = Bit Map
    int FS_Type;
    int Inodes_Count;
    int Blocks_Count;
    int Free_Inodes_Count;
    int Free_Blocks_Count;
    char MTime[16];
    char UMTime[16];
    int Mnt_Count;
    int Magic; //0xEF53 = 61267
    int Inode_Size;
    int Block_Size;
    int First_Ino;
    int First_Blo;
    int BM_Inode_Start;
    int BM_Block_Start;
    int Inode_Start;
    int Block_Start;
} Super_Bloque;

typedef struct {
    char Journal_Tipo_Operacion;
    char Journal_Tipo;
    char Journal_Nombre[500];
    char Journal_Contenido[500];
    char Journal_Fecha[16];
    int Journal_Propietario;
    int Journal_Permisos;
} Journal;

typedef struct {
    int I_UID;
    int I_GID;
    int I_Size;
    int I_ATime[16];
    int I_CTime[16];
    int I_MTime[16];
    int I_Block[15];
    int I_Type;
    int I_Perm;
} Inodo;

typedef struct {
    char B_Name[12];
    int B_Inodo;
} Content;

typedef struct {
    Content B_Content[4];
} Content_Block;

typedef struct {
    char B_Content[64];
} File_Block;

typedef struct {
    char Name[16];
    char Group[16];
    int Permiso_Group;
    int Tipo_Usuario;
    char ID_Particion[16];
    char Path_Disco[300];
    int Start_INodes;
    int Start_Blocks;
    int Size_Particion;
    int ID_User;
    int ID_Grupo;
    char BM_Block[270000];
    char BM_INode[270000];
    Particion_Montada P_Usuario;
} Usuario;

typedef struct {
    char Name[500];
    char Group[500];
    char Password[500];
    int N_Grupo;
    int Posicion;
} Entidad;

typedef struct {
    char Nombre[500];
    char Contenido[280320];
    int Estado;
    int ID_Grupo;
    int ID_Usuario;
    int Permiso;
    int Posicion;
} Archivo;

typedef struct {
    int Puntero[16];
} Apuntador_Bloque;

Disco_Montado Sistema[26];
Usuario USR_Ac;
Entidad Ent_Globales[200];
int NEnt_Globales;

Super_Bloque I_Super_Bloque(Super_Bloque SP) {
    SP.FS_Type = -1;
    SP.Inodes_Count = -1;
    SP.Blocks_Count = -1;
    SP.Free_Inodes_Count = -1;
    SP.Free_Blocks_Count = -1;
    memset(&SP.MTime, 0, 16);
    memset(&SP.UMTime, 0, 16);
    SP.Mnt_Count = -1;
    SP.Magic = -1;
    SP.Inode_Size = -1;
    SP.Block_Size = -1;
    SP.First_Ino = -1;
    SP.First_Blo = -1;
    SP.BM_Inode_Start = -1;
    SP.BM_Block_Start = -1;
    SP.Inode_Start = -1;
    SP.Block_Start = -1;
    return SP;
}

Journal I_Journal(Journal J) {
    J.Journal_Tipo_Operacion = 'x';
    J.Journal_Tipo = 'x';
    memset(&J.Journal_Nombre, 0, 999);
    memset(&J.Journal_Fecha, 0, 16);
    memset(&J.Journal_Contenido, 0, 999);
    J.Journal_Propietario = -1;
    J.Journal_Permisos = -1;
    return J;
}

Inodo I_Inodo(Inodo N) {
    N.I_UID = -1;
    N.I_GID = -1;
    N.I_Size = -1;
    memset(&N.I_ATime[16], 0, 16);
    memset(&N.I_CTime[16], 0, 16);
    memset(&N.I_MTime[16], 0, 16);
    for (int i = 0; i < 15; i++) {
        N.I_Block[i] = -1;
    }
    N.I_Type = 'x';
    N.I_Perm = -1;
    return N;
}

int ST_Inodo(int n, int start) {
    if (start == 0) {
        start = USR_Ac.Start_INodes;
    }
    if (n == -1) {
        return -1;
    }
    return start + (n * sizeof (Inodo));
}

int ST_Bloque(int n, int start) {
    if (start == 0) {
        start = USR_Ac.Start_Blocks;
    }
    if (n == -1) {
        return -1;
    }
    return start + (n * sizeof (File_Block));
}

Content_Block I_Content_Block(Content_Block CB) {
    for (int i = 0; i < 4; i++) {
        memset(&CB.B_Content[i].B_Name, 0, 12);
        CB.B_Content[i].B_Inodo = -1;
    }
    return CB;
}

File_Block initFileBlock(File_Block FB) {
    memset(&FB.B_Content, 0, 64);
    return FB;
}

int I_Usuario() {
    memset(&USR_Ac.Name, 0, 16);
    memset(&USR_Ac.Group, 0, 16);
    memset(&USR_Ac.ID_Particion, 0, 16);
    memset(&USR_Ac.Path_Disco, 0, 300);
    USR_Ac.Permiso_Group = -1;
    USR_Ac.Tipo_Usuario = -1;
    USR_Ac.Start_INodes = -1;
    USR_Ac.Size_Particion = -1;
    USR_Ac.ID_User = -1;
    USR_Ac.ID_Grupo = -1;
    memset(&USR_Ac.BM_Block, 0, 999);
    memset(&USR_Ac.BM_INode, 0, 999);
    return 0;
}

Super_Bloque G_Super_Bloque(Particion_Montada Particion, FILE *Disco) {
    Super_Bloque Aux;
    Aux = I_Super_Bloque(Aux);
    if (Disco != NULL) {
        fseek(Disco, Particion.Start, SEEK_SET);
        fread(&Aux, sizeof (Aux), 1, Disco);
        if (Aux.Magic != 61267 && Aux.Magic != -1) {
            Aux = I_Super_Bloque(Aux);
        }
    }
    return Aux;
}

Archivo G_Arch(char Ruta[], Particion_Montada Particion, int SInodo, FILE* Disco) {
    Archivo A_Final;
    memset(&A_Final.Contenido, 0, 280320);
    A_Final.Estado = -1;
    if (Disco != NULL) {
        Super_Bloque P_SP;
        P_SP = G_Super_Bloque(Particion, Disco);
        if (P_SP.Magic == 61267) {
            Apuntador_Bloque pb1;
            Apuntador_Bloque pb2;
            Apuntador_Bloque pb3;
            File_Block FB_Arch;
            Content_Block BC_Arch;
            Inodo In_Arch;
            fseek(Disco, SInodo, SEEK_SET);
            fread(&In_Arch, sizeof (Inodo), 1, Disco);
            char auxRuta[999] = {0};
            strcpy(auxRuta, Ruta);
            bool encontrado = false;
            int posision = -1;

            for (int j = 0; j < 15; j++) {
                if (encontrado == true) {
                    break;
                }
                if (In_Arch.I_Block[j] == -1) {
                    continue;
                }
                if (j < 12) {
                    fseek(Disco, ST_Bloque(In_Arch.I_Block[j], P_SP.Block_Start), SEEK_SET);
                    fread(&BC_Arch, sizeof (BC_Arch), 1, Disco);
                    for (int a = 0; a < 4; a++) {
                        if (encontrado == true) {
                            break;
                        }
                        if (strcmp(BC_Arch.B_Content[a].B_Name, auxRuta) == 0) {
                            A_Final.Posicion = BC_Arch.B_Content[a].B_Inodo;
                            fseek(Disco, ST_Inodo(BC_Arch.B_Content[a].B_Inodo, P_SP.Inode_Start), SEEK_SET);
                            fread(&In_Arch, sizeof (Inodo), 1, Disco);
                            posision = ST_Inodo(BC_Arch.B_Content[a].B_Inodo, P_SP.Inode_Start);
                            encontrado = true;
                        }
                    }
                } else {
                    fseek(Disco, ST_Bloque(In_Arch.I_Block[j], P_SP.Block_Start), SEEK_SET);
                    fread(&pb1, sizeof (Apuntador_Bloque), 1, Disco);
                    for (int z = 0; z < 16; z++) {
                        if (encontrado == true) {
                            break;
                        }
                        if (pb1.Puntero[z] == -1) {
                            continue;
                        }
                        if (j == 12) {
                            fseek(Disco, ST_Bloque(pb1.Puntero[z], P_SP.Block_Start), SEEK_SET);
                            fread(&BC_Arch, sizeof (BC_Arch), 1, Disco);
                            for (int a = 0; a < 4; a++) {
                                if (encontrado == true) {
                                    break;
                                }
                                if (strcmp(BC_Arch.B_Content[a].B_Name, auxRuta) == 0) {
                                    A_Final.Posicion = BC_Arch.B_Content[a].B_Inodo;
                                    fseek(Disco, ST_Inodo(BC_Arch.B_Content[a].B_Inodo, P_SP.Inode_Start), SEEK_SET);
                                    fread(&In_Arch, sizeof (Inodo), 1, Disco);
                                    posision = ST_Inodo(BC_Arch.B_Content[a].B_Inodo, P_SP.Inode_Start);
                                    encontrado = true;
                                }
                            }
                        } else {
                            fseek(Disco, ST_Bloque(pb1.Puntero[z], P_SP.Block_Start), SEEK_SET);
                            fread(&pb2, sizeof (Apuntador_Bloque), 1, Disco);
                            for (int w = 0; w < 16; w++) {
                                if (encontrado == true) {
                                    break;
                                }
                                if (pb2.Puntero[w] == -1) {
                                    continue;
                                }
                                if (j == 13) {
                                    fseek(Disco, ST_Bloque(pb2.Puntero[w], P_SP.Block_Start), SEEK_SET);
                                    fread(&BC_Arch, sizeof (BC_Arch), 1, Disco);
                                    for (int a = 0; a < 4; a++) {
                                        if (encontrado == true) {
                                            break;
                                        }
                                        if (strcmp(BC_Arch.B_Content[a].B_Name, auxRuta) == 0) {
                                            A_Final.Posicion = BC_Arch.B_Content[a].B_Inodo;
                                            fseek(Disco, ST_Inodo(BC_Arch.B_Content[a].B_Inodo, P_SP.Inode_Start), SEEK_SET);
                                            fread(&In_Arch, sizeof (Inodo), 1, Disco);
                                            posision = ST_Inodo(BC_Arch.B_Content[a].B_Inodo, P_SP.Inode_Start);
                                            encontrado = true;
                                        }
                                    }
                                } else {
                                    fseek(Disco, ST_Bloque(pb2.Puntero[w], P_SP.Block_Start), SEEK_SET);
                                    fread(&pb3, sizeof (Apuntador_Bloque), 1, Disco);
                                    for (int v = 0; v < 16; v++) {
                                        if (encontrado == true) {
                                            break;
                                        }
                                        if (pb3.Puntero[v] == -1) {
                                            continue;
                                        }
                                        fseek(Disco, ST_Bloque(pb3.Puntero[v], P_SP.Block_Start), SEEK_SET);
                                        fread(&BC_Arch, sizeof (BC_Arch), 1, Disco);
                                        for (int a = 0; a < 4; a++) {
                                            if (encontrado == true) {
                                                break;
                                            }
                                            if (strcmp(BC_Arch.B_Content[a].B_Name, auxRuta) == 0) {
                                                A_Final.Posicion = BC_Arch.B_Content[a].B_Inodo;
                                                fseek(Disco, ST_Inodo(BC_Arch.B_Content[a].B_Inodo, P_SP.Inode_Start), SEEK_SET);
                                                fread(&In_Arch, sizeof (Inodo), 1, Disco);
                                                posision = ST_Inodo(BC_Arch.B_Content[a].B_Inodo, P_SP.Inode_Start);
                                                encontrado = true;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (encontrado == true && In_Arch.I_Type == '1') {
                A_Final.Estado = 1;
                A_Final.ID_Usuario = In_Arch.I_UID;
                A_Final.ID_Grupo = In_Arch.I_GID;
                A_Final.Permiso = In_Arch.I_Perm;
                time_t tiempoActual = time(NULL);
                struct tm *fecha = localtime(&tiempoActual);
                strftime(In_Arch.I_ATime, sizeof (In_Arch.I_ATime) - 1, "%d/%m/%y %H:%M", fecha);

                for (int j = 0; j < 15; j++) {
                    if (In_Arch.I_Block[j] == -1) {
                        continue;
                    }
                    if (j < 12) {
                        fseek(Disco, ST_Bloque(In_Arch.I_Block[j], P_SP.Block_Start), SEEK_SET);
                        fread(&FB_Arch, sizeof (File_Block), 1, Disco);
                        sprintf(A_Final.Contenido, "%s%s", A_Final.Contenido, FB_Arch.B_Content);
                    } else {
                        fseek(Disco, ST_Bloque(In_Arch.I_Block[j], P_SP.Block_Start), SEEK_SET);
                        fread(&pb1, sizeof (Apuntador_Bloque), 1, Disco);
                        for (int z = 0; z < 16; z++) {
                            if (pb1.Puntero[z] == -1) {
                                continue;
                            }
                            fseek(Disco, ST_Bloque(pb1.Puntero[z], P_SP.Block_Start), SEEK_SET);
                            if (j == 12) {
                                fread(&FB_Arch, sizeof (File_Block), 1, Disco);
                                sprintf(A_Final.Contenido, "%s%s", A_Final.Contenido, FB_Arch.B_Content);
                            } else {
                                fread(&pb2, sizeof (Apuntador_Bloque), 1, Disco);
                                for (int w = 0; w < 16; w++) {
                                    if (pb2.Puntero[w] == -1) {
                                        continue;
                                    }
                                    fseek(Disco, ST_Bloque(pb2.Puntero[w], P_SP.Block_Start), SEEK_SET);
                                    if (j == 13) {
                                        fread(&FB_Arch, sizeof (File_Block), 1, Disco);
                                        sprintf(A_Final.Contenido, "%s%s", A_Final.Contenido, FB_Arch.B_Content);
                                    } else {
                                        fread(&pb3, sizeof (Apuntador_Bloque), 1, Disco);
                                        for (int v = 0; v < 16; v++) {
                                            if (pb3.Puntero[v] == -1) {
                                                continue;
                                            }
                                            fseek(Disco, ST_Bloque(pb3.Puntero[v], P_SP.Block_Start), SEEK_SET);
                                            fread(&FB_Arch, sizeof (File_Block), 1, Disco);
                                            sprintf(A_Final.Contenido, "%s%s", A_Final.Contenido, FB_Arch.B_Content);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                fseek(Disco, posision, SEEK_SET);
                fwrite(&In_Arch, sizeof (Inodo), 1, Disco);
                sprintf(A_Final.Nombre, "%s", auxRuta);
            }
        }
    }
    return A_Final;
}

int main() {
    int Validacion = 0;
    char Cadena[500] = "";
    ISist();
    I_Usuario();

    while (Validacion != 1) {
        printf("INGRESE EL COMANDO A EJECUTAR: \n");
        fgets(Cadena, 500, stdin);
        Cambiar_Formato(Cadena);
        Validacion = COMANDO(Cadena);
    }
    printf("FIN DEL PROGRAMA. \n");
    return 0;
}

int COMANDO(char Cadena[]) {
    strcat(Cadena, " ");
    int Pos_Ac = 0;
    int Val_Param = -1;
    int Estado = 0;
    //BANDERAS ELEMENTOS ADICIONALES.
    int EX_G = 0;
    int EX_Comando = 0;
    int EX_Coms = -1;
    int EX_DP = 0;
    int EX_PathArch = 0;
    int EX_NameArch = 0;
    //VARIABLES PARA GURDAR DATOS.
    int Val_Size = 0;
    char Val_Fit = 'f';
    int Val_Unit = 0;
    char Val_Path[500] = {0};
    char Val_Type = 'p';
    int Val_Delete = 0;
    int Val_Add = 0;
    char Val_Name[500] = {0};
    char Val_ID[500] = {0};
    char Val_FS[3] = {0};
    int Val_TypeFS = 0;
    char Val_USR[500] = {0};
    char Val_PWD[500] = {0};
    int Prio_FDisk = 0;
    char Aux[1000] = {0};
    //VARIABLES BANDERA
    int EX_Size = 0;
    int EX_Fit = 0;
    int EX_Unit = 0;
    int EX_Path = 0;
    int EX_Type = 0;
    int EX_Delete = 0;
    int EX_Add = 0;
    int EX_Name = 0;
    int EX_ID = 0;
    int EX_FS = 0;
    int EX_USR = 0;
    int EX_PWD = 0;
    while (Cadena[Pos_Ac] != NULL) {
        char AX_Cad[] = {Cadena[Pos_Ac], '\0'};
        if (Cadena[Pos_Ac] == '\r') {
            Cadena[Pos_Ac] = ' ';
        }
        if (((Cadena[Pos_Ac] == 32 || Cadena[Pos_Ac] == '\n') && EX_Coms == -1) || (EX_Coms == 1 && Cadena[Pos_Ac] == 34)) {
            if (EX_Coms == 1 && Cadena[Pos_Ac] == 34) {
                EX_Coms = -1;
            }
            if (EX_Comando == 1) {
                if (Estado == 0) {
                    if (strcmp(Aux, "exit") == 0) {
                        printf("MENSAJE: SE ENCONTRO EL COMANDO EXIT.\n");
                        Estado = 10;
                    } else if (strcmp(Aux, "mkdisk") == 0) {
                        printf("MENSAJE: SE ENCONTRO EL COMANDO MKDISK.\n");
                        Estado = 1;
                    } else if (strcmp(Aux, "rmdisk") == 0) {
                        printf("MENSAJE: SE ENCONTRO EL COMANDO RMDISK.\n");
                        Estado = 2;
                    } else if (strcmp(Aux, "fdisk") == 0) {
                        printf("MENSAJE: SE ENCONTRO EL COMANDO FDISK.\n");
                        Estado = 3;
                    } else if (strcmp(Aux, "mount") == 0) {
                        printf("MENSAJE: SE ENCONTRO EL COMANDO MOUNT.\n");
                        Estado = 4;
                    } else if (strcmp(Aux, "unmount") == 0) {
                        printf("MENSAJE: SE ENCONTRO EL COMANDO UNMOUNT.\n");
                        Estado = 5;
                    } else if (strcmp(Aux, "exec") == 0) {
                        printf("MENSAJE: SE ENCONTRO EL COMANDO EXEC. \n");
                        Estado = 6;
                    } else if (strcmp(Aux, "rep") == 0) {
                        printf("MENSAJE: SE ENCONTRO EL COMANDO REP. \n");
                        Estado = 7;
                    } else if (strcmp(Aux, "mostrar") == 0) {
                        printf("MENSAJE: SE ENCONTRO EL COMANDO MOSTRAR. \n");
                        Estado = 8;
                    } else if (strcmp(Aux, "pause") == 0) {
                        printf("MENSAJE: SE ENCONTRO EL COMANDO PAUSE. \n");
                        Estado = 9;
                    } else if (strcmp(Aux, "mkfs") == 0) {
                        printf("MENSAJE: SE ENCONTRO EL COMANDO MKFS. \n");
                        Estado = 11;
                    } else if (strcmp(Aux, "login") == 0) {
                        printf("MENSAJE: SE ENCONTRO EL COMANDO LOGIN. \n");
                        Estado = 12;
                    } else if (strcmp(Aux, "logout") == 0) {
                        printf("MENSAJE: SE ENCONTRO EL COMANDO LOGOUT. \n");
                        Estado = 13;
                    } else if (strcmp(Aux, "mkgrp") == 0) {
                        printf("MENSAJE: SE ENCONTRO EL COMANDO MKGRP. \n");
                        Estado = 14;
                    } else {
                        printf("ERROR: EL COMANDO '%s' NO ES VALIDO. \n", Aux);
                        return 0;
                    }
                    *Aux = '\0';
                } else if (strcmp(Aux, "") != 0) {
                    if (Val_Param == 0) {
                        printf("ERROR: EL PARAMETRO '%s' NO ES VALIDO. \n", Aux);
                    } else if (Val_Param == 1) {
                        if (EX_Size == 1) {
                            printf("ERROR: YA HAY UN VALOR ASIGNADO AL PARAMETRO SIZE. \n");
                        } else {
                            EX_Size = 1;
                            Val_Size = atoi(Aux);
                            printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO SIZE: %i. \n", Val_Size);
                        }
                    } else if (Val_Param == 2) {
                        if (EX_Fit == 1) {
                            printf("ERROR: YA HAY UN VALOR ASIGNADO AL PARAMETRO FIT.\n");
                        } else {
                            EX_Fit = 1;
                            if (strcmp(Aux, "bf") == 0) {
                                Val_Fit = 'b';
                                printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO FIT: %c. \n", Val_Fit);
                            } else if (strcmp(Aux, "ff") == 0) {
                                Val_Fit = 'f';
                                printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO FIT: %c. \n", Val_Fit);
                            } else if (strcmp(Aux, "wf") == 0) {
                                Val_Fit = 'w';
                                printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO FIT: %c. \n", Val_Fit);
                            } else {
                                printf("ERROR: EL VALOR %s NO ES VALIDO PARA EL PARAMETRO FIT. \n", Aux);
                            }
                        }
                    } else if (Val_Param == 3) {
                        if (EX_Unit == 1) {
                            printf("ERROR: YA HAY UN VALOR ASIGNADO AL PARAMETRO UNIT. \n");
                        } else {
                            EX_Unit = 1;
                            if (strcmp(Aux, "b") == 0) {
                                Val_Unit = 1;
                                printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO UNIT: %s. \n", Aux);
                            } else if (strcmp(Aux, "k") == 0) {
                                Val_Unit = 1024;
                                printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO UNIT: %s. \n", Aux);
                            } else if (strcmp(Aux, "m") == 0) {
                                Val_Unit = 1024 * 1024;
                                printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO UNIT: %s. \n", Aux);
                            }
                        }
                    } else if (Val_Param == 4) {
                        if (EX_Path == 1) {
                            printf("ERROR: YA HAY UN VALOR ASIGNADO AL PARAMETRO PATH. \n");
                        } else {
                            EX_Path = 1;
                            EX_PathArch = -1;
                            strcpy(Val_Path, &Aux);
                            printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO PATH: %s. \n", Val_Path);
                            *Aux = '\0';
                        }
                    } else if (Val_Param == 5) {
                        if (EX_Type == 1) {
                            printf("ERROR: YA HAY UN VALOR ASIGNADO AL PARAMETRO TYPE. \n");
                        } else {
                            EX_Type = 1;
                            if (strcmp(Aux, "p") == 0) {
                                Val_Type = 'p';
                                printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO TYPE: %c. \n", Val_Type);
                            } else if (strcmp(Aux, "e") == 0) {
                                Val_Type = 'e';
                                printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO TYPE: %c. \n", Val_Type);
                            } else if (strcmp(Aux, "l") == 0) {
                                Val_Type = 'l';
                                printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO TYPE: %c. \n", Val_Type);
                            } else if (strcmp(Aux, "full") == 0) {
                                Val_TypeFS = 1;
                                printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO TYPE: %s. \n", Aux);
                            } else if (strcmp(Aux, "fast") == 0) {
                                Val_TypeFS = 2;
                                printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO TYPE: %s. \n", Aux);
                            } else {
                                printf("ERROR: EL VALOR %s NO ES VALIDO PARA EL PARAMETRO TYPE. \n", Aux);
                            }
                        }
                    } else if (Val_Param == 6) {
                        if (EX_Delete == 1) {
                            printf("ERROR: YA HAY UN VALOR ASIGNADO AL PARAMETRO DELETE. \n");
                        } else {
                            if (strcmp(Aux, "full") == 0) {
                                Val_Delete = 1;
                                printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO DELETE: %s. \n", Aux);
                            } else if (strcmp(Aux, "fast") == 0) {
                                Val_Delete = 2;
                                printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO DELETE: %s. \n", Aux);
                            } else {
                                printf("ERROR: EL VALOR %s NO ES VALIDO PARA EL PARAMETRO DELETE. \n", Aux);
                            }
                            if (Val_Delete != 0 && Prio_FDisk == 0) {
                                EX_Delete = 1;
                                Prio_FDisk = 2;
                            }
                        }
                    } else if (Val_Param == 7) {
                        if (EX_Add == 1) {
                            printf("ERROR: YA HAY UN VALOR ASIGNADO AL PARAMETRO ADD. \n");
                        } else {
                            Val_Add = atoi(Aux);
                            printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO ADD: %i. \n", Val_Add);
                            if (Prio_FDisk == 0) {
                                EX_Add = 1;
                                Prio_FDisk = 1;
                            }
                        }
                    } else if (Val_Param == 8) {
                        if (EX_Name == 1) {
                            printf("ERROR: YA HAY UN VALOR ASIGNADO AL PARAMETRO NAME. \n");
                        } else {
                            EX_Name = 1;
                            EX_NameArch = -1;
                            strcpy(Val_Name, &Aux);
                            printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO NAME: %s. \n", Val_Name);
                        }
                    } else if (Val_Param == 9) {
                        if (EX_ID == 1) {
                            printf("ERROR: YA HAY UN VALOR ASIGNADO AL PARAMETRO ID. \n");
                        } else {
                            EX_ID = 1;
                            strcpy(Val_ID, &Aux);
                            printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO ID: %s. \n", Val_ID);
                        }
                    } else if (Val_Param == 10) {
                        if (EX_FS == 1) {
                            printf("ERROR: YA HAY UN VALOR ASIGNADO AL PARAMETRO FS. \n");
                        } else {
                            EX_FS = 1;
                            strcpy(Val_FS, Aux);
                            printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO FS: %s. \n", Val_FS);
                        }
                    } else if (Val_Param == 11) {
                        if (EX_USR == 1) {
                            printf("ERROR: YA HAY UN VALOR ASIGNADO AL PARAMETRO USR. \n");
                        } else {
                            EX_USR = 1;
                            strcpy(Val_USR, Aux);
                            printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO USR: %s. \n", Val_USR);
                        }
                    } else if (Val_Param == 12) {
                        if (EX_PWD == 1) {
                            printf("ERROR: YA HAY UN VALOR ASIGNADO AL PARAMETRO PWD. \n");
                        } else {
                            EX_PWD = 1;
                            strcpy(Val_PWD, Aux);
                            printf("MENSAJE: SE GUARDO EL VALOR DEL PARAMETRO PWD: %s. \n", Val_PWD);
                        }
                    }
                    Val_Param = 0;
                    *Aux = '\0';
                }
                if (Cadena[Pos_Ac] == 35) {
                    char Com[] = {Cadena[Pos_Ac], '\0'};
                    while (Cadena[Pos_Ac] != NULL) {
                        strcat(Com, Cadena[Pos_Ac]);
                        Pos_Ac++;
                    }
                    printf("MENSAJE: COMENTARIO ENCONTRADO. \n");
                    printf("CONTENIDO: %s. \n", Com);
                    break;
                }
            }
        } else {
            EX_Comando = 1;
            if (Cadena[Pos_Ac] == 34) {
                EX_Coms = 1;
            } else {
                strcat(Aux, AX_Cad);
                AX_Cad[0] = '\0';
            }
            if (Cadena[Pos_Ac] == 45 && EX_DP == 0 && EX_G == 0) {
                EX_G = 1;
            } else if (Cadena[Pos_Ac] == 58 && EX_DP == 0 && EX_G == 1) {
                EX_DP = 1;
            } else if (Cadena[Pos_Ac] == 126 && EX_DP == 1 && EX_G == 1) {
                if (strcmp(Aux, "-size~:~") == 0) {
                    printf("MENSAJE: SE ENCONTRO EL PARAMETRO SIZE.\n");
                    Val_Param = 1;
                } else if (strcmp(Aux, "-fit~:~") == 0) {
                    printf("MENSAJE: SE ENCONTRO EL PARAMETRO FIT.\n");
                    Val_Param = 2;
                } else if (strcmp(Aux, "-unit~:~") == 0) {
                    printf("MENSAJE: SE ENCONTRO EL PARAMETRO UNIT.\n");
                    Val_Param = 3;
                } else if (strcmp(Aux, "-path~:~") == 0) {
                    printf("MENSAJE: SE ENCONTRO EL PARAMETRO PATH\n");
                    EX_PathArch = 1;
                    Val_Param = 4;
                } else if (strcmp(Aux, "-type~:~") == 0) {
                    printf("MENSAJE: SE ENCONTRO EL PARAMETRO TYPE.\n");
                    Val_Param = 5;
                } else if (strcmp(Aux, "-delete~:~") == 0) {
                    printf("MENSAJE: SE ENCONTRO EL PARAMETRO DELETE.\n");
                    Val_Param = 6;
                } else if (strcmp(Aux, "-add~:~") == 0) {
                    printf("MENSAJE: SE ENCONTRO EL PARAMETRO ADD.\n");
                    Val_Param = 7;
                } else if (strcmp(Aux, "-name~:~") == 0) {
                    printf("MENSAJE: SE ENCONTRO EL PARAMETRO NAME.\n");
                    Val_Param = 8;
                    EX_NameArch = 1;
                } else if (strcmp(Aux, "-id~:~") == 0) {
                    printf("MENSAJE: SE ENCONTRO EL PARAMETRO ID.\n");
                    Val_Param = 9;
                } else if (strcmp(Aux, "-fs~:~") == 0) {
                    printf("MENSAJE: SE ENCONTRO EL PARAMETRO FS.\n");
                    Val_Param = 10;
                } else if (strcmp(Aux, "-usr~:~") == 0) {
                    printf("MENSAJE: SE ENCONTRO EL PARAMETRO USR.\n");
                    Val_Param = 11;
                } else if (strcmp(Aux, "-pwd~:~") == 0) {
                    printf("MENSAJE: SE ENCONTRO EL PARAMETRO FS.\n");
                    Val_Param = 12;
                } else {
                    Val_Param = 0;
                    printf("ERROR: EL PARAMETRO '%s' NO PERTENECE A NINGUN COMANDO.\n", Aux);
                    return 0;
                }
                EX_G = 0;
                EX_DP = 0;
                *Aux = '\0';
            }
        }
        Pos_Ac++;
    }
    if (Estado == 1) {
        if (EX_Size == 1 && EX_Path == 1) {
            if (EX_ID == 1 || EX_Add == 1 || EX_Name == 1 || EX_Delete == 1 || EX_Type == 1 || EX_FS == 1 || EX_USR == 1 || EX_PWD == 1) {
                printf("ERROR: HAY PARAMETROS QUE NO PERTENECEN AL COMANDO MKDISK. \n");
            } else {
                if (EX_Unit == 0) {
                    Val_Unit = 1024 * 1024;
                }
                if (EX_Fit == 0) {
                    Val_Fit = 'f';
                }
                if (EX_Unit == 1 && Val_Unit == 1) {
                    printf("ERROR: EL PARAMETRO UNIT TIENE UN VALOR ERRONEO (BITS). \n");
                } else {
                    Val_Size = Val_Size * Val_Unit;
                    MBR A;
                    for (int i = 0; i < 4; i++) {
                        A.Particiones[i].Status = 'I';
                        A.Particiones[i].Start = -1;
                        A.Particiones[i].Size = 0;
                        A.Particiones[i].Type = 'p';
                        A.Particiones[i].Fit = 'f';
                        strcpy(A.Particiones[i].Name, "");
                    }
                    if (Val_Size > 0) {
                        MKDISK(Val_Path, Val_Size, Val_Fit, A);
                    } else {
                        printf("ERROR: EL TAMAÑO DEL DISCO NO PUEDE SER UN NUMERO NEGATIVO. \n");
                    }
                }
            }
        } else {
            if (EX_Size == 0) {
                printf("ERROR: NO SE ENCONTRO EL PARAMETRO SIZE PARA EJECUTAR EL COMANDO MKDISK.\n");
            }
            if (EX_Path == 0) {
                printf("ERROR: NO SE ENCONTRO EL PARAMETRO PATH PARA EJECUTAR EL COMANDO MKDISK.\n");
            }
        }
    } else if (Estado == 2) {
        if (EX_Path == 1) {
            if (EX_Size == 1 || EX_Unit == 1 || EX_Type == 1 || EX_Fit == 1 || EX_Delete == 1 || EX_Name == 1 || EX_Add == 1 || EX_ID == 1 || EX_FS == 1 || EX_USR == 1 || EX_PWD == 1) {
                printf("ERROR: HAY PARAMETROS QUE NO PARTENECEN AL COMANDO RMDISK. \n");
            } else {
                if (Validar_Ex(Val_Path) == 1) {\
                    RMDISK(Val_Path);
                } else {
                    printf("ERROR: EL DISCO SELECCIONADO NO EXISTE. \n");
                }
            }
        } else {
            printf("ERROR: NO SE ENCONTRO EL PARAMETRO PATH PARA EJECUTAR EL COMANDO RMDISK. \n");
        }
    } else if (Estado == 3) {
        if (EX_Path == 1 && EX_Name == 1 || EX_FS == 1 || EX_USR == 1 || EX_PWD == 1) {
            if (EX_ID == 1) {
                printf("ERROR: HAY PARAMETROS QUE NO PARTENECEN AL COMANDO FDISK. \n");
            } else {
                if (EX_Unit == 0) {
                    Val_Unit = 1024;
                }
                if (EX_Type == 0) {
                    Val_Type = 'p';
                }
                if (EX_Fit == 0) {
                    Val_Fit = 'w';
                }
                Val_Size = Val_Size * Val_Unit;
                Val_Add = Val_Add * Val_Unit;
                FDISK(Val_Path, Val_Name, Val_Size, Val_Type, Val_Fit, Val_Delete, Val_Add, Prio_FDisk);
            }
        } else {
            if (EX_Path == 0) {
                printf("ERROR: NO SE ENCONTRO EL PARAMETRO PATH PARA EJECUTAR EL COMANDO FDISK. \n");
            }
            if (EX_Name == 0) {
                printf("ERROR:  NO SE ENCONTRO EL PARAMETRO NAME PARA EJECUTAR EL COMANDO FDISK \n");
            }
        }
    } else if (Estado == 4) {
        if (EX_Path == 1 && EX_Name == 1) {
            if (EX_Size == 1 || EX_Unit == 1 || EX_Type == 1 || EX_Fit == 1 || EX_Delete == 1 || EX_Add == 1 || EX_ID == 1 || EX_FS == 1 || EX_USR == 1 || EX_PWD == 1) {
                printf("ERROR: HAY PARAMETROS QUE NO PARTENECEN AL COMANDO MOUNT. \n");
            } else {
                MOUNT(Val_Path, Val_Name);
            }
        } else {
            if (EX_Path == 0) {
                printf("ERROR: NO SE ENCONTRO EL PARAMETRO PATH PARA EJECUTAR EL COMANDO MOUNT. \n");
            }
            if (EX_Name == 0) {
                printf("ERROR: NO SE ENCONTRO EL PARAMETRO NAME PARA EJECUTAR EL COMANDO MOUNT. \n");
            }
        }
    } else if (Estado == 5) {
        if (EX_ID == 1) {
            if (EX_Size == 1 || EX_Unit == 1 || EX_Path == 1 || EX_Type == 1 || EX_Fit == 1 || EX_Delete == 1 || EX_Name == 1 || EX_Add == 1 || EX_FS == 1 || EX_USR == 1 || EX_PWD == 1) {
                printf("ERROR: HAY PARAMETROS QUE NO PARTENECEN AL COMANDO UNMOUNT. \n");
            } else {
                UNMOUNT(Val_ID);
            }
        } else {
            if (EX_ID == 0) {
                printf("ERROR: NO SE ENCONTRO EL PARAMETRO PATH EJECUTAR EL COMANDO UNMOUNT. \n");
            }
        }
    } else if (Estado == 6) {
        if (EX_Path == 1) {
            if (EX_Size == 1 || EX_Unit == 1 || EX_Type == 1 || EX_Fit == 1 || EX_Delete == 1 || EX_Name == 1 || EX_Add == 1 || EX_ID == 1 || EX_FS == 1 || EX_USR == 1 || EX_PWD == 1) {
                printf("ERROR: HAY PARAMETROS QUE NO PERTENECEN AL COMANDO EXEC. \n");
            } else {
                EXEC(Val_Path);
            }
        } else {
            printf("ERROR:  NO SE ENCONTRO EL PARAMETRO PATH PARA EJECUTAR EL COMANDO EXEC.\n");
        }
    } else if (Estado == 7) {
        if (EX_ID == 1 && EX_Path == 1 && EX_Name == 1) {
            if (EX_Size == 1 || EX_Unit == 1 || EX_Type == 1 || EX_Fit == 1 || EX_Delete == 1 || EX_Add == 1 || EX_FS == 1 || EX_USR == 1 || EX_PWD == 1) {
                printf("ERROR: HAY PARAMETROS QUE NO PERTENECEN AL COMANDO REP. \n");
            } else {
                int Disco = -1;
                char Ruta[500] = {0};
                for (int i = 0; i < 26; i++) {
                    if (Sistema[i].letra != -1) {
                        for (int j = 0; j < 10; j++) {
                            if (Sistema[i].Particiones[j].Start != -1) {
                                if (strcmp(Sistema[i].Particiones[j].ID, Val_ID) == 0) {
                                    strcpy(Ruta, Sistema[i].Path);
                                    Disco = 1;
                                    break;
                                }
                            }
                        }
                    }
                    if (Disco == 1) {
                        break;
                    }
                }
                char Com_Consola[500] = {0};
                char Ext_Arch[100] = {0};
                for (int i = 0; i < strlen(Val_Path); i++) {
                    sprintf(Ext_Arch, "%s%c", Ext_Arch, Val_Path[i]);
                    if (Val_Path[i] == '.') {
                        sprintf(Ext_Arch, "");
                    }
                }
                Generar_Archivo(Val_Path);
                char Destino[] = {"/home/Cod_Reporte.dot"};
                int Tipo = -1;
                sprintf(Com_Consola, "dot -T%s %s -o %s", Ext_Arch, Destino, Val_Path);
                if (strcmp(Val_Name, "mbr") == 0) {
                    Tipo = 1;
                } else if (strcmp(Val_Name, "disk") == 0) {
                    Tipo = 2;
                } else {
                    printf("ERROR: NO SE ENCONTRO UN PARAMETRO VALIDO PARA GENERAR EL REPORTE. \n");
                    return 0;
                }
                if (REP(Destino, Ruta, Tipo) == 1) {
                    if (Validar_Ex(Val_Path) == 1) {
                        remove(Val_Path);
                    }
                    system(Com_Consola);
                    if (Validar_Ex(Val_Path) == 1) {
                        printf("SE GENERO CON EXITO EL REPORTE. \n");
                    } else {
                        printf("ERROR: NO SE PUDO GENERAR EL REPORTE. \n");
                    }
                } else {
                    printf("ERROR: NO SE PUDO GENERAR EL REPORTE. \n");
                }
            }
        } else {
            if (EX_ID == 0) {
                printf("ERROR: NO SE ENCONTRO EL PARAMETRO ID PARA EJECUTAR  EL COMANDO REP.\n");
            }
            if (EX_Path == 0) {
                printf("ERROR: NO SE ENCONTRO EL PARAMETRO PATH PARA EJECUTAR EL COMANDO REP.\n");
            }
            if (EX_Name == 0) {
                printf("ERROR: NO SE ENCONTRO EL PARAMETRO PATH PARA EJECUTAR.\n");
            }
        }
    } else if (Estado == 8) {
        printf("DISCOS MONTADOS EN EL SISTEMA: \n");
        for (int i = 0; i < 26; i++) {
            if (Sistema[i].letra != -1) {
                printf("\tDisco '%c' : path '%s' Numero de Particiones:\n", Sistema[i].letra, Sistema[i].Path);
                for (int j = 0; j < 10; j++) {
                    if (Sistema[i].Particiones[j].Start != -1) {
                        printf("\t\tParticion '%s' : Nombre '%s' \n", Sistema[i].Particiones[j].ID, Sistema[i].Particiones[j].Name);
                    }
                }
            }
        }
    } else if (Estado == 9) {
        char Cadena[500] = "";
        printf("PRESIONE LA TECLA ENTER PARA CONTINUAR: \n");
        fgets(Cadena, 500, stdin);
    } else if (Estado == 10) {
        printf("SE SALDRA DEL PROGRAMA. \n");
        return 1;
    } else if (Estado == 11) {
        if (EX_ID == 1) {
            if (EX_Size == 1 || EX_Unit == 1 || EX_Path == 1 || EX_Fit == 1 || EX_Delete == 1 || EX_Name == 1 || EX_Add == 1 || EX_USR == 1 || EX_PWD == 1 /* bgrp == 1 || bugo == 1 || br == 1 || bp == 1 || bcont == 1 || bfilen == 1 || bdest == 1 || bruta == 1*/) {
                printf("ERROR: HAY PARAMETROS QUE NO PERTENECEN AL COMANDO FS. \n");
            } else {
                int VFS = 3;
                if (EX_Type == 0) {
                    Val_TypeFS = 1;
                }
                if (Val_Type == 1 || Val_Type == 2) {
                    int Particion_Encontrada = -1;
                    Particion_Montada Part_Pivote;
                    memset(Part_Pivote.ID, 0, 5);
                    Part_Pivote.Fit = 'x';
                    memset(Part_Pivote.Name, 0, 16);
                    Part_Pivote.Size = -1;
                    Part_Pivote.Type = 'x';
                    Part_Pivote.Start = -1;
                    Part_Pivote.Status = 'x';

                    char Path_Aux[999] = {0};
                    for (int i = 0; i < 26; i++) {
                        for (int j = 0; j < 10; j++) {
                            if (strcmp(Val_ID, Sistema[i].Particiones[j].ID) == 0) {
                                Part_Pivote = Sistema[i].Particiones[j];
                                strcpy(Path_Aux, Sistema[i].Path);
                                Particion_Encontrada = 1;
                                break;
                            }
                        }
                        if (Particion_Encontrada == 1) {
                            break;
                        }
                    }
                    if (Particion_Encontrada == 1) {
                        if (USR_Ac.ID_User != -1 && USR_Ac.ID_Grupo != -1) {
                            printf("ERROR: HAY UNA SESION ACTIVA, NO SE PUEDE FORMATEAR LA PARTICION.\n");
                        } else {
                            MKFS(Path_Aux, Part_Pivote, Val_Type, VFS);
                        }
                    } else {
                        printf("ERROR: LA PARTICION BUSCADA NO ESTA MONTADA. \n");
                    }
                } else {
                    printf("ERROR: HAY UN VALOR INCORRECTO DEL PARAMETRO TYPE. \n");
                }
            }
        } else {
            if (EX_ID == 0) {
                printf("ERROR:  NO SE ENCONTRO EL PARAMETRO ID PARA EJECUTAR EL COMANDO MKFS.\n");
            }
        }
    } else if (Estado == 12) {
        if (EX_ID == 1 && EX_USR == 1 && EX_PWD == 1) {
            if (EX_Size == 1 || EX_Unit == 1 || EX_Path == 1 || EX_Type == 1 || EX_Fit == 1 || EX_Delete == 1 || EX_Name == 1 || EX_Add == 1 || EX_FS == 1) {
                printf("ERROR: HAY PARAMETROS QUE NO PERTENECEN AL COMANDO LOGIN. \n");
            } else {
                int Particion_Encontrada = -1;
                Particion_Montada Part_Buscada;
                memset(Part_Buscada.ID, 0, 5);
                Part_Buscada.Fit = 'x';
                memset(Part_Buscada.Name, 0, 16);
                Part_Buscada.Size = -1;
                Part_Buscada.Type = 'x';
                Part_Buscada.Start = -1;
                Part_Buscada.Status = 'x';

                char Ruta[999] = {0};
                for (int i = 0; i < 26; i++) {
                    for (int j = 0; j < 10; j++) {
                        if (strcmp(Val_ID, Sistema[i].Particiones[j].ID) == 0) {
                            Part_Buscada = Sistema[i].Particiones[j];
                            strcpy(Ruta, Sistema[i].Path);
                            Particion_Encontrada = 1;
                            break;
                        }
                    }
                    if (Particion_Encontrada == 1) {
                        break;
                    }
                }
                if (Particion_Encontrada == 1) {
                    if (USR_Ac.ID_User == -1 && USR_Ac.ID_Grupo == -1) {
                        FILE *Disco = fopen(Ruta, "rb+");
                        if (Disco != NULL) {
                            LOGIN(Val_USR, Val_PWD, Ruta, Part_Buscada, Disco);
                            // actualizarSP(USR_Ac.partUser, 0, Disco);
                        }
                    } else {
                        printf("ERROR: UN USUARIO YA HA REALIZADO EL LOGIN EN EL SISTEMA .\n");
                    }
                } else {
                    printf("ERROR: LA PARTICION BUSCADA NO EXISTE. \n");
                }
            }
        } else {
            if (EX_ID == 0) {
                printf("ERROR:  NO SE ENCONTRO EL PARAMETRO ID PARA EJECUTAR EL COMANDO LOGIN.\n");
            }
            if (EX_USR == 0) {
                printf("ERROR:  NO SE ENCONTRO EL PARAMETRO USR PARA EJECUTAR EL COMANDO LOGIN.\n");
            }
            if (EX_PWD == 0) {
                printf("ERROR:  NO SE ENCONTRO EL PARAMETRO PWD PARA EJECUTAR EL COMANDO LOGIN.\n");
            }
        }
    } else if (Estado == 13) {
        if (EX_Size == 1 || EX_Unit == 1 || EX_Type == 1 || EX_Fit == 1 || EX_Delete == 1 || EX_Name == 1 || EX_Path == 1 || EX_Add == 1 || EX_ID == 1 || EX_FS == 1 || EX_USR == 1 || EX_PWD == 1) {
            printf("ERROR: HAY PARAMETROS QUE NO PERTENECEN AL COMANDO LOGOUT. \n");
        } else {
            if (USR_Ac.ID_User != -1) {
                I_Usuario();
            } else {
                printf("ERROR: NO HAY NINGUN USUARIO INGRESADO EN EL SISTEMA ACTUALMENTE. \n");
            }
        }
    }else if (Estado == 14) {
        if (EX_Size == 1 || EX_Unit == 1 || EX_Type == 1 || EX_Fit == 1 || EX_Delete == 1 || EX_Path == 1 || EX_Add == 1 || EX_ID == 1 || EX_FS == 1 || EX_USR == 1 || EX_PWD == 1) {
            printf("ERROR: HAY PARAMETROS QUE NO PERTENECEN AL COMANDO MKGRP. \n");
        } else {
            if (EX_Name == 1) {
                I_Usuario();
            } else {
                printf("ERROR: NO HAY NINGUN USUARIO INGRESADO EN EL SISTEMA ACTUALMENTE. \n");
            }
        }
    }
    return 0;
}

void MKDISK(char Path[], int Size, char Fit, MBR Conte) {
    if (Validar_Ex(Path) == 1) {
        printf("ERROR: YA EXISTE UN DISCO CON ESTE PATH. \n");
    } else {
        char Ruta[500] = {0};
        strcpy(Ruta, Path);
        Generar_Archivo(Path);
        if (Validar_Extension(Path, ".disk") == 0) {
            FILE *Disco_Nuevo;
            Disco_Nuevo = fopen(Ruta, "wb");
            if (Disco_Nuevo == NULL) {
                printf("ERROR: NO SE PUDO GENERAR EL DISCO. \n");
            } else {
                MBR Aux;
                Aux.ID = 201504325;
                Aux.Size = Size;
                Aux.Fit = Fit;
                time_t tiempoActual = time(NULL);
                struct tm *fecha = localtime(&tiempoActual);
                strftime(Aux.FCreacion, sizeof (Aux.FCreacion) - 1, "%d/%m/%y %H:%M:%S", fecha);
                for (int i = 0; i < 4; i++) {
                    Aux.Particiones[i].Status = Conte.Particiones[i].Status;
                    Aux.Particiones[i].Start = Conte.Particiones[i].Start;
                    Aux.Particiones[i].Fit = Conte.Particiones[i].Fit;
                    Aux.Particiones[i].Type = Conte.Particiones[i].Type;
                    strcpy(Aux.Particiones[i].Name, Conte.Particiones[i].Name);
                    Aux.Particiones[i].Size = Conte.Particiones[i].Size;
                }
                fwrite(&Aux, sizeof (Aux), 1, Disco_Nuevo);
                int Val_Pivote = 1024 * 1024;
                if (Size > Val_Pivote) {
                    char Contenido[1024 * 1024] = {0};
                    while (Size > Val_Pivote) {
                        Size = Size - Val_Pivote;
                        fwrite(Contenido, sizeof (char), Val_Pivote, Disco_Nuevo);
                    }
                } else {
                    char Contenido[Size];
                    for (int i = 0; i < Size; i++) {
                        Contenido[i] = '\0';
                    }
                    fwrite(&Contenido, sizeof (char), Size, Disco_Nuevo);
                }
                fclose(Disco_Nuevo);
                printf("SE GENERO CON EXITO EL DISCO. \n");
            }
        } else {
            printf("ERORR: NO SE PUDO GENERAR EL DISCO. \n");
        }
    }
}

void RMDISK(char Path[]) {
    if (Validar_Extension(Path, ".disk") == 0) {
        int ResEL = remove(Path);
        if (ResEL == 0) {
            printf("SE ELIMINO CON EXITO EL DISCO. \n");
        } else {
            printf("ERROR: NO SE PUDO ELIMINAR EL DISCO. \n");
        }
    } else {
        printf("ERROR: NO SE PUDO ELIMINAR EL DISCO. \n");
    }

}

void FDISK(char path[], char name[], int size, char type, char fit, int del, int add, int tipoFDisk) {
    if (Validar_Ex(path) == 1 && Validar_Extension(path, ".disk") == 0) {
        MBR Aux_MBR;
        int found = 0;
        int inicio_disco = 140;
        int tope_particion = 0;

        FILE *Disco_Usado;
        Disco_Usado = fopen(path, "rb+");
        if (Disco_Usado != NULL) {
            int auxPos = -1;
            fread(&Aux_MBR, sizeof (Aux_MBR), 1, Disco_Usado);
            tope_particion = Aux_MBR.Size;
            Particion auxiliar_partition;
            int partition_found = -1;
            int extended_found = -1;
            int cantidad_primarias = 0;

            for (int i = 0; i < 4; i++) {
                if (Aux_MBR.Particiones[i].Start != -1) {
                    if (strcmp(name, Aux_MBR.Particiones[i].Name) == 0) {
                        auxPos = i;
                        partition_found = 1;
                    }
                }
                if (Aux_MBR.Particiones[i].Type == 'e') {
                    extended_found = 1;
                    auxiliar_partition = Aux_MBR.Particiones[i];
                }
                if (Aux_MBR.Particiones[i].Type == 'p' && Aux_MBR.Particiones[i].Start != -1) {
                    cantidad_primarias++;
                }
            }

            if (partition_found == -1 && extended_found == 1) {
                int ebr_encontrado = 0;
                EBR Aux_EBR;
                EBR Aux_EBR_2;
                fseek(Disco_Usado, auxiliar_partition.Start, SEEK_SET);
                fread(&Aux_EBR, sizeof (Aux_EBR), 1, Disco_Usado);
                Aux_EBR_2 = Aux_EBR;
                if (Aux_EBR.Start != -1 || Aux_EBR.Next != -1) {
                    if (strcmp(Aux_EBR.Name, name) == 0) {
                        partition_found = 2;
                        ebr_encontrado = 1;
                    } else {
                        while (Aux_EBR.Next != -1) {
                            fseek(Disco_Usado, Aux_EBR.Next, SEEK_SET);
                            Aux_EBR_2 = Aux_EBR;
                            fread(&Aux_EBR, sizeof (Aux_EBR), 1, Disco_Usado);
                            if (strcmp(Aux_EBR.Name, name) == 0) {
                                partition_found = 2;
                                ebr_encontrado = 1;
                                break;
                            }
                        }
                    }
                }

                if (type == 'l' || partition_found == 2) {
                    if (partition_found != -1) {

                        if (tipoFDisk == 0) {
                            printf("ERROR: YA EXISTE UNA PARTICION LOGICA CON EL NOMBRE '%s' \n", name);
                        } else if (tipoFDisk == 1) {

                            int tope_EBR = Aux_EBR.Next;
                            if (Aux_EBR.Next == -1) {

                                tope_EBR = auxiliar_partition.Start + auxiliar_partition.Size;
                            }
                            int final_ebr = Aux_EBR.Size + add;
                            if (Aux_EBR.Start + final_ebr < tope_EBR && final_ebr > 32) {
                                Aux_EBR.Size = final_ebr;
                                fseek(Disco_Usado, Aux_EBR.Start, SEEK_SET);
                                fwrite(&Aux_EBR, sizeof (Aux_EBR), 1, Disco_Usado);
                                printf("SE CAMBIO EL TAMAÑO DE LA PARTICION '%s' DE '%d' A '%d' \n", name, add, final_ebr);
                            } else {
                                printf("ERROR: NO SE PUEDE USAR EL COMANDO ADD EN ESTA PARTICION. \n");
                            }
                        } else if (tipoFDisk == 2) {

                            int principio = Aux_EBR.Start;
                            int sizeReal = Aux_EBR.Size;
                            if (del == 1) {

                                if (Aux_EBR.Start == Aux_EBR_2.Start) {

                                    principio = principio + 32;
                                    sizeReal = sizeReal - 32;
                                    fseek(Disco_Usado, Aux_EBR_2.Start, SEEK_SET);
                                    Aux_EBR_2.Status = 'n';
                                    Aux_EBR_2.Start = -1;
                                    Aux_EBR_2.Size = -1;
                                    memset(&Aux_EBR_2.Name, 0, sizeof (Aux_EBR_2.Name));
                                    fwrite(&Aux_EBR_2, sizeof (Aux_EBR_2), 1, Disco_Usado);
                                    printf("SE ELIMINO LA PARTICION LOGICA CON EXITO. \n");
                                } else {

                                    Aux_EBR_2.Next = Aux_EBR.Next;
                                    fseek(Disco_Usado, Aux_EBR_2.Start, SEEK_SET);
                                    fwrite(&Aux_EBR_2, sizeof (Aux_EBR_2), 1, Disco_Usado);
                                    printf("SE ELIMINO LA PARTICION LOGICA CON EXITO . \n");
                                }

                                fseek(Disco_Usado, principio, SEEK_SET);
                                int sizeMax = 1024 * 1024;
                                if (sizeReal > sizeMax) {
                                    char insert[1024 * 1024] = {0};
                                    while (sizeReal > sizeMax) {
                                        sizeReal = sizeReal - sizeMax;
                                        fwrite(insert, sizeof (char), sizeMax, Disco_Usado);
                                    }
                                }

                                char ultimoInsert[sizeReal];
                                for (int i = 0; i < sizeReal; i++) {
                                    ultimoInsert[i] = '\0';
                                }
                                fwrite(&ultimoInsert, sizeof (char), sizeReal, Disco_Usado);
                            } else if (del == 2) {

                                if (Aux_EBR.Start == Aux_EBR_2.Start) {

                                    fseek(Disco_Usado, Aux_EBR_2.Start, SEEK_SET);
                                    Aux_EBR_2.Status = 'n';
                                    Aux_EBR_2.Start = -1;
                                    Aux_EBR_2.Size = -1;
                                    memset(&Aux_EBR_2.Name, 0, sizeof (Aux_EBR_2.Name));
                                    fwrite(&Aux_EBR_2, sizeof (Aux_EBR_2), 1, Disco_Usado);
                                    printf("SE ELIMINO LA PARTICION LOGICA CON EXITO. \n");
                                } else {

                                    Aux_EBR_2.Next = Aux_EBR.Next;
                                    fseek(Disco_Usado, Aux_EBR_2.Start, SEEK_SET);
                                    fwrite(&Aux_EBR_2, sizeof (Aux_EBR_2), 1, Disco_Usado);
                                    printf("SE ELIMINO LA PARTICION LOGICA CON EXITO. \n");
                                }
                            } else {
                                //Algo salio mal
                                printf("ERROR:  PARAMETROS INCORRECTOS ENCONTRADOS.\n");
                            }

                        } else {
                            //Algo salio mal
                            printf("ERROR: PARAMETROS INCORRECTOS ENCONTRADOS. \n");
                        }
                    } else {
                        EBR insercion_primero;
                        EBR insercion_aux;
                        fseek(Disco_Usado, auxiliar_partition.Start, SEEK_SET);
                        fread(&insercion_primero, sizeof (insercion_primero), 1, Disco_Usado);
                        if (insercion_primero.Next != -1 || insercion_primero.Start != -1) {

                            char fit_Extendida = auxiliar_partition.Fit;
                            int exito_logica = -1;
                            if (fit_Extendida == 'f') {

                                while (insercion_primero.Next != -1) {
                                    if (insercion_primero.Next - (insercion_primero.Start + insercion_primero.Size) > 0) {

                                        if (size <= insercion_primero.Next - (insercion_primero.Start + insercion_primero.Size)) {
                                            EBR EBR_Nuevo;

                                            if (insercion_primero.Start == -1 && insercion_primero.Size == -1) {
                                                EBR_Nuevo.Start = auxiliar_partition.Start;
                                            } else {
                                                EBR_Nuevo.Start = insercion_primero.Size + insercion_primero.Start;
                                            }

                                            EBR_Nuevo.Fit = fit;
                                            EBR_Nuevo.Next = insercion_primero.Next;
                                            EBR_Nuevo.Size = size;
                                            EBR_Nuevo.Status = 'a';

                                            memset(&EBR_Nuevo.Name, 0, sizeof (EBR_Nuevo.Name));
                                            strcpy(&EBR_Nuevo.Name, name);

                                            if (insercion_primero.Start != -1) {
                                                insercion_primero.Next = EBR_Nuevo.Start;
                                                fseek(Disco_Usado, insercion_primero.Start, SEEK_SET);
                                                fwrite(&insercion_primero, sizeof (insercion_primero), 1, Disco_Usado);

                                                fseek(Disco_Usado, EBR_Nuevo.Start, SEEK_SET);
                                                fwrite(&EBR_Nuevo, sizeof (EBR_Nuevo), 1, Disco_Usado);

                                                exito_logica = 1;
                                                printf("SE CREO LA PARTICION LOGICA CON EXITO. \n");
                                                break;
                                            } else {
                                                fseek(Disco_Usado, EBR_Nuevo.Start, SEEK_SET);
                                                fwrite(&EBR_Nuevo, sizeof (EBR_Nuevo), 1, Disco_Usado);

                                                exito_logica = 1;
                                                printf("SE CREO LA PARTICION LOGICA CON EXITO. \n");
                                                break;
                                            }
                                        }
                                    }
                                    if (insercion_primero.Next == -1) {
                                        EBR EBR_Nuevo;

                                        EBR_Nuevo.Start = insercion_primero.Size + insercion_primero.Start;
                                        EBR_Nuevo.Fit = fit;
                                        EBR_Nuevo.Next = -1;
                                        EBR_Nuevo.Size = size;
                                        EBR_Nuevo.Status = 'a';

                                        memset(&EBR_Nuevo.Name, 0, sizeof (EBR_Nuevo.Name));
                                        strcpy(&EBR_Nuevo.Name, name);

                                        insercion_primero.Next = EBR_Nuevo.Start;
                                        fseek(Disco_Usado, insercion_primero.Start, SEEK_SET);
                                        fwrite(&insercion_primero, sizeof (insercion_primero), 1, Disco_Usado);

                                        fseek(Disco_Usado, EBR_Nuevo.Start, SEEK_SET);
                                        fwrite(&EBR_Nuevo, sizeof (EBR_Nuevo), 1, Disco_Usado);

                                        exito_logica = 1;
                                        printf("SE CREO LA PARTICION LOGICA CON EXITO. \n");
                                        break;
                                    }
                                    insercion_aux = insercion_primero;
                                    fseek(Disco_Usado, insercion_primero.Next, SEEK_SET);
                                    fread(&insercion_primero, sizeof (insercion_primero), 1, Disco_Usado);
                                }
                                if (insercion_primero.Next == -1 && exito_logica == -1) {
                                    EBR EBR_Nuevo;

                                    EBR_Nuevo.Start = insercion_primero.Size + insercion_primero.Start;
                                    EBR_Nuevo.Fit = fit;
                                    EBR_Nuevo.Next = -1;
                                    EBR_Nuevo.Size = size;
                                    EBR_Nuevo.Status = 'a';

                                    memset(&EBR_Nuevo.Name, 0, sizeof (EBR_Nuevo.Name));
                                    strcpy(&EBR_Nuevo.Name, name);

                                    insercion_primero.Next = EBR_Nuevo.Start;
                                    fseek(Disco_Usado, insercion_primero.Start, SEEK_SET);
                                    fwrite(&insercion_primero, sizeof (insercion_primero), 1, Disco_Usado);

                                    fseek(Disco_Usado, EBR_Nuevo.Start, SEEK_SET);
                                    fwrite(&EBR_Nuevo, sizeof (EBR_Nuevo), 1, Disco_Usado);

                                    printf("SE CREO LA PARTICION LOGICA CON EXITO. \n");
                                } else {

                                }
                            } else if (fit_Extendida == 'b') {
                                EBR mejor_fit;
                                mejor_fit.Next = 999999999;
                                mejor_fit.Size = 0;
                                mejor_fit.Start = 0;

                                while (insercion_primero.Next != -1) {
                                    if (insercion_primero.Next - (insercion_primero.Start + insercion_primero.Size) > 0) {

                                        if (size <= insercion_primero.Next - (insercion_primero.Start + insercion_primero.Size)) {
                                            if ((insercion_primero.Next - insercion_primero.Start - insercion_primero.Size) < (mejor_fit.Next - mejor_fit.Start - mejor_fit.Size)) {
                                                mejor_fit = insercion_primero;
                                                exito_logica = 1;
                                            }
                                        }
                                    }
                                    if (insercion_primero.Next == -1) {
                                        //Insertar al final
                                        if (size <= auxiliar_partition.Start + auxiliar_partition.Size - insercion_primero.Start - insercion_primero.Size) {
                                            //Espacio libre entre la ultima particion y el final de la extendida
                                            if ((auxiliar_partition.Start + auxiliar_partition.Size - insercion_primero.Start - insercion_primero.Size) < (mejor_fit.Next - mejor_fit.Start - mejor_fit.Size)) {
                                                //Ultima particion logica
                                                mejor_fit = insercion_primero;
                                                exito_logica = 1;
                                            }
                                        }
                                    }
                                    insercion_aux = insercion_primero;
                                    fseek(Disco_Usado, insercion_primero.Next, SEEK_SET);
                                    fread(&insercion_primero, sizeof (insercion_primero), 1, Disco_Usado);
                                }
                                if (insercion_primero.Next == -1) {
                                    if (size <= auxiliar_partition.Start + auxiliar_partition.Size - insercion_primero.Start - insercion_primero.Size) {
                                        if ((auxiliar_partition.Start + auxiliar_partition.Size - insercion_primero.Start - insercion_primero.Size) < (mejor_fit.Next - mejor_fit.Start - mejor_fit.Size)) {
                                            mejor_fit = insercion_primero;
                                            exito_logica = 1;
                                        }
                                    }
                                }
                                if (exito_logica == 1) {
                                    EBR EBR_Nuevo;

                                    EBR_Nuevo.Start = mejor_fit.Start + mejor_fit.Size;
                                    EBR_Nuevo.Fit = fit;
                                    EBR_Nuevo.Status = 'a';
                                    EBR_Nuevo.Size = size;
                                    EBR_Nuevo.Next = mejor_fit.Next;

                                    memset(&EBR_Nuevo.Name, 0, sizeof (EBR_Nuevo.Name));
                                    strcpy(&EBR_Nuevo.Name, name);

                                    mejor_fit.Next = EBR_Nuevo.Start;
                                    fseek(Disco_Usado, mejor_fit.Start, SEEK_SET);
                                    fwrite(&mejor_fit, sizeof (mejor_fit), 1, Disco_Usado);

                                    fseek(Disco_Usado, EBR_Nuevo.Start, SEEK_SET);
                                    fwrite(&EBR_Nuevo, sizeof (EBR_Nuevo), 1, Disco_Usado);

                                    printf("SE CREO LA PARTICION LOGICA CON EXITO. \n");
                                } else {

                                }
                            } else if (fit_Extendida == 'w') {

                                EBR peor_fit;
                                peor_fit.Next = 0;
                                peor_fit.Size = 0;
                                peor_fit.Start = 0;

                                while (insercion_primero.Next != -1) {
                                    if (insercion_primero.Next - (insercion_primero.Start + insercion_primero.Size) > 0) {
                                        if (size <= insercion_primero.Next - (insercion_primero.Start + insercion_primero.Size)) {
                                            if ((insercion_primero.Next - insercion_primero.Start - insercion_primero.Size) > (peor_fit.Next - peor_fit.Start - peor_fit.Size)) {
                                                peor_fit = insercion_primero;
                                                exito_logica = 1;
                                            }
                                        }
                                    }
                                    if (insercion_primero.Next == -1) {

                                        if (size <= auxiliar_partition.Start + auxiliar_partition.Size - insercion_primero.Start - insercion_primero.Size) {

                                            if ((auxiliar_partition.Start + auxiliar_partition.Size - insercion_primero.Start - insercion_primero.Size) > (peor_fit.Next - peor_fit.Start - peor_fit.Size)) {

                                                peor_fit = insercion_primero;
                                                exito_logica = 1;
                                            }
                                        }
                                    }
                                    insercion_aux = insercion_primero;
                                    fseek(Disco_Usado, insercion_primero.Next, SEEK_SET);
                                    fread(&insercion_primero, sizeof (insercion_primero), 1, Disco_Usado);
                                }
                                if (insercion_primero.Next == -1) {

                                    if (size <= auxiliar_partition.Start + auxiliar_partition.Size - insercion_primero.Start - insercion_primero.Size) {
                                        if ((auxiliar_partition.Start + auxiliar_partition.Size - insercion_primero.Start - insercion_primero.Size) > (peor_fit.Next - peor_fit.Start - peor_fit.Size)) {

                                            peor_fit = insercion_primero;
                                            exito_logica = 1;
                                        }
                                    }
                                }
                                if (exito_logica == 1) {

                                    EBR EBR_Nuevo;

                                    EBR_Nuevo.Start = peor_fit.Start + peor_fit.Size;
                                    EBR_Nuevo.Fit = fit;
                                    EBR_Nuevo.Status = 'a';
                                    EBR_Nuevo.Size = size;
                                    EBR_Nuevo.Next = peor_fit.Next;

                                    memset(&EBR_Nuevo.Name, 0, sizeof (EBR_Nuevo.Name));
                                    strcpy(&EBR_Nuevo.Name, name);

                                    peor_fit.Next = EBR_Nuevo.Start;
                                    fseek(Disco_Usado, peor_fit.Start, SEEK_SET);
                                    fwrite(&peor_fit, sizeof (peor_fit), 1, Disco_Usado);

                                    fseek(Disco_Usado, EBR_Nuevo.Start, SEEK_SET);
                                    fwrite(&EBR_Nuevo, sizeof (EBR_Nuevo), 1, Disco_Usado);

                                    printf("SE CREO LA PARTICION LOGICA CON EXITO. \n");
                                } else {

                                }
                            } else {

                            }
                        } else {

                            int tope_EBR = auxiliar_partition.Size;
                            if (insercion_primero.Next != -1) {

                                tope_EBR = insercion_primero.Next;
                            }
                            if (size > tope_EBR) {
                                printf("ERROR: NO HAY SUFICIENTE ESPACIO PARA CREAR LA PARTICION LOGICA. \n");
                            } else {

                                EBR EBR_Nuevo;

                                EBR_Nuevo.Start = auxiliar_partition.Start;
                                EBR_Nuevo.Fit = fit;
                                EBR_Nuevo.Next = -1;
                                EBR_Nuevo.Size = size;
                                EBR_Nuevo.Status = 'a';
                                memset(&EBR_Nuevo.Name, 0, sizeof (EBR_Nuevo.Name));
                                strcpy(&EBR_Nuevo.Name, name);

                                fseek(Disco_Usado, auxiliar_partition.Start, SEEK_SET);
                                fwrite(&EBR_Nuevo, sizeof (EBR_Nuevo), 1, Disco_Usado);

                                printf("SE CREO LA PARTICION LOGICA CON EXITO. \n");
                            }
                        }
                    }
                }
            }
            if (type == 'l' && extended_found != 1) {
                printf("ERROR: NO HAY UNA PARTICION EXTENDIDA PARA CREAR LA PARTICION LOGICA. \n");
            } else if (partition_found == -1 && type != 'l') {

                if (size > 0) {

                    char fit_disco = Aux_MBR.Fit;
                    int auxPos2 = -1;

                    if (fit_disco == 'f') {

                        for (int i = 0; i < 4; i++) {
                            auxPos2 = i;
                            if (Aux_MBR.Particiones[i].Start == -1) {
                                break;
                            }
                            if (i == 0) {
                                int disponible = Aux_MBR.Particiones[i].Start - 140;
                                if (disponible >= size) {
                                    inicio_disco = 140;
                                    found = 1;
                                }
                            } else {
                                int disponible = Aux_MBR.Particiones[i].Start - Aux_MBR.Particiones[i - 1].Start - Aux_MBR.Particiones[i - 1].Size;
                                if (disponible >= size) {
                                    inicio_disco = Aux_MBR.Particiones[i - 1].Start + Aux_MBR.Particiones[i - 1].Size;
                                    found = 1;
                                }
                            }
                            if (found == 0) {
                                inicio_disco = Aux_MBR.Particiones[i].Start + Aux_MBR.Particiones[i].Size;
                            }
                        }
                    } else if (fit_disco == 'b') {

                        int posiciones[5] = {0};
                        int espacios[5] = {0};
                        for (int i = 0; i < 4; i++) {
                            auxPos2 = i;
                            if (Aux_MBR.Particiones[i].Start == -1) {

                                break;
                            }
                            if (i == 0) {
                                int disponible = Aux_MBR.Particiones[i].Start - 140;
                                espacios[i] = disponible;
                                posiciones[i] = i;
                                if (Aux_MBR.Particiones[i + 1].Start == -1) {
                                    espacios[i + 1] = tope_particion - Aux_MBR.Particiones[i].Start - Aux_MBR.Particiones[i].Size;
                                    posiciones[i + 1] = i + 1;
                                } else {
                                    espacios[i + 1] = Aux_MBR.Particiones[i + 1].Start - Aux_MBR.Particiones[i].Start - Aux_MBR.Particiones[i].Size;
                                    posiciones[i + 1] = i + 1;
                                }
                            } else {
                                int disponible = Aux_MBR.Particiones[i].Start - Aux_MBR.Particiones[i - 1].Start - Aux_MBR.Particiones[i - 1].Size;
                                espacios[i] = disponible;
                                posiciones[i] = i;
                                if (Aux_MBR.Particiones[i + 1].Start == -1 || i == 3) {
                                    espacios[i + 1] = tope_particion - Aux_MBR.Particiones[i].Start - Aux_MBR.Particiones[i].Size;
                                    posiciones[i + 1] = i + 1;
                                } else {
                                    espacios[i + 1] = Aux_MBR.Particiones[i + 1].Start - Aux_MBR.Particiones[i].Start - Aux_MBR.Particiones[i].Size;
                                    posiciones[i + 1] = i + 1;
                                }
                            }
                        }
                        int posAjuste = -1;
                        int mejorAjuste = 999999999;
                        for (int i = 0; i < 5; i++) {
                            if (espacios[i] < mejorAjuste && espacios[i] >= size) {
                                mejorAjuste = espacios[i];
                                posAjuste = i;
                            }
                        }
                        if (posAjuste != -1) {
                            if (posAjuste == 0) {

                                inicio_disco = 140;
                            } else if (posAjuste == 1) {

                                inicio_disco = Aux_MBR.Particiones[0].Start + Aux_MBR.Particiones[0].Size;
                            } else if (posAjuste == 2) {

                                inicio_disco = Aux_MBR.Particiones[1].Start + Aux_MBR.Particiones[1].Size;
                            } else if (posAjuste == 3) {

                                inicio_disco = Aux_MBR.Particiones[2].Start + Aux_MBR.Particiones[2].Size;
                            } else { //posAjuste == 4

                                inicio_disco = Aux_MBR.Particiones[3].Start + Aux_MBR.Particiones[3].Size;
                            }
                        } else {
                            if (auxPos2 == 0 && Aux_MBR.Particiones[auxPos2].Start == -1) {
                                inicio_disco = 140;
                            } else {
                                if (Aux_MBR.Particiones[auxPos2].Start == -1) {
                                    inicio_disco = Aux_MBR.Particiones[auxPos2 - 1].Start + Aux_MBR.Particiones[auxPos2 - 1].Size;
                                } else {
                                    inicio_disco = Aux_MBR.Particiones[auxPos2].Start + Aux_MBR.Particiones[auxPos2].Size;
                                }
                            }
                        }
                    } else if (fit_disco == 'w') {

                        int posiciones[5] = {0};
                        int espacios[5] = {0};
                        for (int i = 0; i < 4; i++) {
                            auxPos2 = i;
                            if (Aux_MBR.Particiones[i].Start == -1) {
                                //Encontro particion vacia
                                break;
                            }
                            if (i == 0) {
                                int disponible = Aux_MBR.Particiones[i].Start - 140;
                                espacios[i] = disponible;
                                posiciones[i] = i;
                                if (Aux_MBR.Particiones[i + 1].Start == -1) {
                                    espacios[i + 1] = tope_particion - Aux_MBR.Particiones[i].Start - Aux_MBR.Particiones[i].Size;
                                    posiciones[i + 1] = i + 1;
                                } else {
                                    espacios[i + 1] = Aux_MBR.Particiones[i + 1].Start - Aux_MBR.Particiones[i].Start - Aux_MBR.Particiones[i].Size;
                                    posiciones[i + 1] = i + 1;
                                }
                            } else {
                                int disponible = Aux_MBR.Particiones[i].Start - Aux_MBR.Particiones[i - 1].Start - Aux_MBR.Particiones[i - 1].Size;
                                espacios[i] = disponible;
                                posiciones[i] = i;
                                if (Aux_MBR.Particiones[i + 1].Start == -1 || i == 3) {
                                    espacios[i + 1] = tope_particion - Aux_MBR.Particiones[i].Start - Aux_MBR.Particiones[i].Size;
                                    posiciones[i + 1] = i + 1;
                                } else {
                                    espacios[i + 1] = Aux_MBR.Particiones[i + 1].Start - Aux_MBR.Particiones[i].Start - Aux_MBR.Particiones[i].Size;
                                    posiciones[i + 1] = i + 1;
                                }
                            }
                        }
                        int posAjuste = -1;
                        int peorAjuste = 0;
                        for (int i = 0; i < 5; i++) {
                            if (espacios[i] > peorAjuste && espacios[i] >= size) {
                                peorAjuste = espacios[i];
                                posAjuste = i;
                            }
                        }
                        if (posAjuste != -1) {
                            if (posAjuste == 0) {

                                inicio_disco = 140;
                            } else if (posAjuste == 1) {

                                inicio_disco = Aux_MBR.Particiones[0].Start + Aux_MBR.Particiones[0].Size;
                            } else if (posAjuste == 2) {

                                inicio_disco = Aux_MBR.Particiones[1].Start + Aux_MBR.Particiones[1].Size;
                            } else if (posAjuste == 3) {

                                inicio_disco = Aux_MBR.Particiones[2].Start + Aux_MBR.Particiones[2].Size;
                            } else {

                                inicio_disco = Aux_MBR.Particiones[3].Start + Aux_MBR.Particiones[3].Size;
                            }
                        } else {
                            if (auxPos2 == 0 && Aux_MBR.Particiones[auxPos2].Start == -1) {
                                inicio_disco = 140;
                            } else {
                                if (Aux_MBR.Particiones[auxPos2].Start == -1) {
                                    inicio_disco = Aux_MBR.Particiones[auxPos2 - 1].Start + Aux_MBR.Particiones[auxPos2 - 1].Size;
                                } else {
                                    inicio_disco = Aux_MBR.Particiones[auxPos2].Start + Aux_MBR.Particiones[auxPos2].Size;
                                }
                            }
                        }
                    }
                    if (auxPos2 == 4) {
                        printf("ERROR: EL DISCO SELECCIONADO YA TIENE TODAS SUS PARTICIONES. \n");
                    } else {
                        if (tope_particion >= inicio_disco + size) {
                            if (cantidad_primarias == 4 && type == 'p') {
                                printf("ERROR: EL DISCO YA TIENE TODAS SUS PARTICIONES. \n");
                            } else {
                                if (extended_found == 1 && type == 'e') {
                                    printf("ERROR: EL DISCO YA TIENE UNA PARTICION EXTENDIDS. \n");
                                } else {
                                    Aux_MBR.Particiones[auxPos2].Fit = fit;
                                    Aux_MBR.Particiones[auxPos2].Size = size;
                                    memset(&Aux_MBR.Particiones[auxPos2].Name, 0, sizeof (Aux_MBR.Particiones[auxPos2].Name));
                                    strcpy(&Aux_MBR.Particiones[auxPos2].Name, name);
                                    Aux_MBR.Particiones[auxPos2].Start = inicio_disco;
                                    Aux_MBR.Particiones[auxPos2].Status = 'a';
                                    Aux_MBR.Particiones[auxPos2].Type = type;
                                    if (type == 'e') {
                                        EBR Aux_EBR;
                                        Aux_EBR.Start = -1;
                                        Aux_EBR.Next = -1;
                                        Aux_EBR.Status = 'n';
                                        fseek(Disco_Usado, inicio_disco, SEEK_SET);
                                        fwrite(&Aux_EBR, sizeof (Aux_EBR), 1, Disco_Usado);
                                    }

                                    printf("SE CREO LA PARTICION CON EXITO. \n");
                                }
                            }
                        } else {
                            printf("ERROR: TAMAÑO INSUFICIENTE PARA CREAR LA PARTICION. \n");
                        }
                    }
                } else if (tipoFDisk == 0) {
                    printf("ERROR: TAMAÑO NO VALIDO. \n");
                } else {

                }
            } else if (partition_found == 1) {

                if (tipoFDisk == 0) {
                    printf("ERROR: YA EXISTE ESTA PARTICION EN ESTE DISCO. \n");
                } else if (tipoFDisk == 1) {

                    int posicion_final = Aux_MBR.Particiones[auxPos].Start + Aux_MBR.Particiones[auxPos].Size + add;
                    tope_particion = Aux_MBR.Size;
                    if (auxPos < 4) {
                        if (Aux_MBR.Particiones[auxPos + 1].Start != -1) {
                            tope_particion = Aux_MBR.Particiones[auxPos + 1].Start;
                        }
                    }
                    if (posicion_final < tope_particion && posicion_final > Aux_MBR.Particiones[auxPos].Start) {
                        Aux_MBR.Particiones[auxPos].Size = Aux_MBR.Particiones[auxPos].Size + add;
                        printf("EXITO: SE CAMBIO EL TAMAÑO DE LA PARTICION '%s' DE  '%d' A '%d' \n", name, add, Aux_MBR.Particiones[auxPos].Size);
                    } else {
                        printf("ERROR: NO SE PUDO USAR EL ADD EN ESTA PARTICION. \n");
                    }
                } else {

                    if (del == 1) {

                        fseek(Disco_Usado, Aux_MBR.Particiones[auxPos].Start, SEEK_SET);
                        int sizeMax = 1024 * 1024;
                        int sizeExtendida = Aux_MBR.Particiones[auxPos].Size;
                        if (sizeExtendida > sizeMax) {
                            char insert[1024 * 1024] = {0};
                            while (sizeExtendida > sizeMax) {
                                sizeExtendida = sizeExtendida - sizeMax;
                                fwrite(insert, sizeof (char), sizeMax, Disco_Usado);
                            }
                        }
                        char ultimoInsert[sizeExtendida];
                        for (int i = 0; i < sizeExtendida; i++) {
                            ultimoInsert[i] = '\0';
                        }
                        fwrite(&ultimoInsert, sizeof (char), sizeExtendida, Disco_Usado);
                        Aux_MBR.Particiones[auxPos].Start = -1;
                        Aux_MBR.Particiones[auxPos].Status = 'n';
                        memset(&Aux_MBR.Particiones[auxPos].Name, 0, sizeof (Aux_MBR.Particiones[auxPos].Name));
                        printf("SE ELIMINO LA PARTICION '%s' . \n", name);
                    } else if (del == 2) {

                        Aux_MBR.Particiones[auxPos].Start = -1;
                        Aux_MBR.Particiones[auxPos].Status = 'n';
                        memset(&Aux_MBR.Particiones[auxPos].Name, 0, sizeof (Aux_MBR.Particiones[auxPos].Name));
                        printf("SE ELIMINO '%s' \n", name);
                    } else {

                    }

                }
            }

            Particion auxiliar_orden;
            for (int i = 1; i < 4; i++) {
                for (int j = 0; j < 4 - i; j++) {
                    if (((Aux_MBR.Particiones[j].Start > Aux_MBR.Particiones[j + 1].Start) && Aux_MBR.Particiones[j + 1].Start != -1) || Aux_MBR.Particiones[j].Start == -1) {
                        auxiliar_orden = Aux_MBR.Particiones[j + 1];
                        Aux_MBR.Particiones[j + 1] = Aux_MBR.Particiones[j];
                        Aux_MBR.Particiones[j] = auxiliar_orden;
                    }
                }
            }

            fseek(Disco_Usado, 0, SEEK_SET);
            fwrite(&Aux_MBR, sizeof (Aux_MBR), 1, Disco_Usado);
            fclose(Disco_Usado);
        } else {
            printf("ERROR: NO SE PUDO ABRIR EL DISCO. \n");
        }
    } else {
        printf("ERROR: EL DISCO SELECCIONADO NO EXISTE. \n");
    }
}

void MOUNT(char Path[], char Name[]) {
    if (Validar_Ex(Path) == 1 && Validar_Extension(Path, ".disk") == 0) {
        int UDisco = -1;
        int EDisco = -1;
        for (int i = 0; i < 26; i++) {
            if (strcmp(Sistema[i].Path, Path) == 0) {
                EDisco = 1;
                UDisco = i;
                break;
            }
            if (Sistema[i].letra == -1) {
                UDisco = i;
                break;
            }
        }
        if (EDisco == 1) {
            FILE *ADisco;
            ADisco = fopen(Path, "rb+");
            if (ADisco != NULL) {
                int UM = -1;
                int EM = -1;
                for (int i = 0; i < 10; i++) {
                    if (strcmp(Sistema[UDisco].Particiones[i].Name, Name) == 0) {
                        EM = 1;
                        break;
                    }
                    if (Sistema[UDisco].Particiones[i].Start == -1) {
                        UM = i;
                        break;
                    }
                }
                MBR MBR_Aux;
                fread(&MBR_Aux, sizeof (MBR_Aux), 1, ADisco);
                Particion AP;
                int ExPart = -1;
                int EPart = -1;
                for (int i = 0; i < 4; i++) {
                    if (MBR_Aux.Particiones[i].Start != -1) {
                        if (strcmp(MBR_Aux.Particiones[i].Name, Name) == 0) {
                            AP = MBR_Aux.Particiones[i];
                            ExPart = i;
                            break;
                        }
                        if (MBR_Aux.Particiones[i].Type == 'e') {
                            EPart = i;
                        }
                    }
                }
                if (EM != 1) {
                    if (ExPart != -1) {
                        if (AP.Type != 'l') {
                            if (UM < 10) {
                                Sistema[UDisco].Particiones[UM].Status = AP.Status;
                                Sistema[UDisco].Particiones[UM].Type = AP.Type;
                                Sistema[UDisco].Particiones[UM].Fit = AP.Fit;
                                Sistema[UDisco].Particiones[UM].Start = AP.Start;
                                Sistema[UDisco].Particiones[UM].Size = AP.Size;
                                Sistema[UDisco].Particiones[UM].ID[0] = 'v';
                                Sistema[UDisco].Particiones[UM].ID[1] = 'd';
                                Sistema[UDisco].Particiones[UM].ID[2] = (char) Sistema[UDisco].letra;
                                Sistema[UDisco].Particiones[UM].ID[3] = (char) 48 + UM + 1;
                                Sistema[UDisco].Particiones[UM].ID[4] = '\0';
                                strcat(&Sistema[UDisco].Particiones[UM].Name, &AP.Name);
                                printf("SE MONTO LA PARTICION %s EL SISTEMA. \n", AP.Name);
                            } else {
                                printf("ERROR: NO SE PUEDE MONTAR LA PARTICION. \n");
                            }
                        } else {
                            printf("ERROR: NO SE PUEDE MONTAR LA PARTICION \n");
                        }
                    } else {
                        if (EPart != -1) {
                            EBR AEBR;
                            int ELog = -1;
                            fseek(ADisco, MBR_Aux.Particiones[EPart].Start, SEEK_SET);
                            fread(&AEBR, sizeof (AEBR), 1, ADisco);
                            if (strcmp(AEBR.Name, Name) == 0) {
                                ELog = 1;
                            }
                            while (AEBR.Next != -1) {
                                if (strcmp(AEBR.Name, Name) == 0) {
                                    ELog = 1;
                                    break;
                                }
                                fseek(ADisco, AEBR.Next, SEEK_SET);
                                fread(&AEBR, sizeof (AEBR), 1, ADisco);
                            }
                            if (ELog == 1) {
                                Sistema[UDisco].Particiones[UM].Status = AEBR.Status;
                                Sistema[UDisco].Particiones[UM].Type = 'l';
                                Sistema[UDisco].Particiones[UM].Fit = AEBR.Fit;
                                Sistema[UDisco].Particiones[UM].Start = AEBR.Start;
                                Sistema[UDisco].Particiones[UM].Size = AEBR.Size;
                                Sistema[UDisco].Particiones[UM].ID[0] = 'v';
                                Sistema[UDisco].Particiones[UM].ID[1] = 'd';
                                Sistema[UDisco].Particiones[UM].ID[2] = (char) Sistema[UDisco].letra;
                                Sistema[UDisco].Particiones[UM].ID[3] = (char) 48 + UM + 1;
                                Sistema[UDisco].Particiones[UM].ID[4] = '\0';
                                strcat(&Sistema[UDisco].Particiones[UM].Name, &AEBR.Name);
                                printf("SE MONTO LA PARTICION %s EL SISTEMA. \n", AP.Name);
                            } else {
                                printf("ERROR: LA PARTICION NO EXISTE. \n");
                            }
                        } else {
                            printf("ERROR: LA PARTICION NO EXISTE. \n");
                        }
                    }
                } else {
                    printf("ERROR: LA PARTICION YA ESTA MONTADA EN EL SISTEMA. \n ");
                }
            } else {
                printf("ERROR: NO SE PUDO ABRIR EL DISCO.\n");
            }
        } else {
            FILE *ADisco;
            ADisco = fopen(Path, "rb+");
            if (ADisco != NULL) {
                int UM = -1;
                for (int i = 0; i < 10; i++) {
                    if (Sistema[UDisco].Particiones[i].Start == -1) {
                        UM = i;
                        break;
                    }
                }
                MBR AMBR;
                fread(&AMBR, sizeof (AMBR), 1, ADisco);
                Particion AP;
                int ExPart = -1;
                int EPart = -1;
                for (int i = 0; i < 4; i++) {
                    if (AMBR.Particiones[i].Start != -1) {
                        if (strcmp(AMBR.Particiones[i].Name, Name) == 0) {
                            AP = AMBR.Particiones[i];
                            ExPart = i;
                            break;
                        }
                        if (AMBR.Particiones[i].Type == 'e') {
                            EPart = i;
                        }
                    }
                }
                if (ExPart != -1) {
                    if (AP.Type != 'l') {
                        if (UM < 10) {
                            Sistema[UDisco].letra = 97 + UDisco;
                            strcpy(&Sistema[UDisco].Path, Path);
                            Sistema[UDisco].Particiones[UM].Status = AP.Status;
                            Sistema[UDisco].Particiones[UM].Type = AP.Type;
                            Sistema[UDisco].Particiones[UM].Fit = AP.Fit;
                            Sistema[UDisco].Particiones[UM].Start = AP.Start;
                            Sistema[UDisco].Particiones[UM].Size = AP.Size;
                            Sistema[UDisco].Particiones[UM].ID[0] = 'v';
                            Sistema[UDisco].Particiones[UM].ID[1] = 'd';
                            Sistema[UDisco].Particiones[UM].ID[2] = (char) Sistema[UDisco].letra;
                            Sistema[UDisco].Particiones[UM].ID[3] = (char) 48 + UM + 1;
                            Sistema[UDisco].Particiones[UM].ID[4] = '\0';
                            strcat(&Sistema[UDisco].Particiones[UM].Name, &AP.Name);
                            printf("SE MONTO LA PARTICION %s EL SISTEMA. \n", AP.Name);
                        } else {
                            printf("ERROR: NO SE PUDO MONTAR LA PARTICION \n");
                        }
                    } else {
                        printf("ERROR: NO SE PUDO MONTAR LA PARTICION \n");
                    }
                } else {
                    if (EPart != -1) {
                        EBR AEBR;
                        int ELog = -1;
                        fseek(ADisco, AMBR.Particiones[EPart].Start, SEEK_SET);
                        fread(&AEBR, sizeof (AEBR), 1, ADisco);
                        if (strcmp(AEBR.Name, Name) == 0) {
                            ELog = 1;
                        }
                        while (AEBR.Next != -1) {
                            if (strcmp(AEBR.Name, Name) == 0) {
                                ELog = 1;
                                break;
                            }
                            fseek(ADisco, AEBR.Next, SEEK_SET);
                            fread(&AEBR, sizeof (AEBR), 1, ADisco);
                        }
                        if (ELog == 1) {
                            Sistema[UDisco].letra = 97 + UDisco;
                            strcpy(&Sistema[UDisco].Path, Path);
                            Sistema[UDisco].Particiones[UM].Status = AEBR.Status;
                            Sistema[UDisco].Particiones[UM].Type = 'l';
                            Sistema[UDisco].Particiones[UM].Fit = AEBR.Fit;
                            Sistema[UDisco].Particiones[UM].Start = AEBR.Start;
                            Sistema[UDisco].Particiones[UM].Size = AEBR.Size;
                            Sistema[UDisco].Particiones[UM].ID[0] = 'v';
                            Sistema[UDisco].Particiones[UM].ID[1] = 'd';
                            Sistema[UDisco].Particiones[UM].ID[2] = (char) Sistema[UDisco].letra;
                            Sistema[UDisco].Particiones[UM].ID[3] = (char) 48 + UM + 1;
                            Sistema[UDisco].Particiones[UM].ID[4] = '\0';
                            strcat(&Sistema[UDisco].Particiones[UM].Name, &AEBR.Name);
                            printf("SE MONTO LA PARTICION %s EL SISTEMA. \n", AP.Name);
                        } else {
                            printf("ERROR: NO SE MONTO LA PARTICION\n");
                        }
                    } else {
                        printf("ERROR: NO SE MONTO LA PARTICION \n");
                    }
                }
            } else {
                printf("ERROR: NO SE PUDO ABRIR EL DISCO. \n");
            }
        }
    } else {
        printf("ERROR: NO EXISTE EL DISCO. \n");
    }
}

void UNMOUNT(char ID[]) {
    int EMount = -1;
    int EXMount = -1;
    for (int i = 0; i < 26; i++) {
        if (Sistema[i].letra != -1) {
            int cantidad_mounts = 0;
            for (int j = 0; j < 10; j++) {
                if (strcmp(Sistema[i].Particiones[j].ID, ID) == 0) {
                    EXMount = 1;
                    memset(&Sistema[i].Particiones[j].Name, 0, 16);
                    memset(&Sistema[i].Particiones[j].ID, 0, 5);
                    Sistema[i].Particiones[j].Status = 'I';
                    Sistema[i].Particiones[j].Type = 'P';
                    Sistema[i].Particiones[j].Fit = 'F';
                    Sistema[i].Particiones[j].Start = -1;
                    Sistema[i].Particiones[j].Size = -1;
                    EMount = 1;
                    printf("ESE DESMONTO LA PARTICION.\n");
                    break;
                }
            }
            for (int j = 0; j < 10; j++) {
                if (Sistema[i].Particiones[j].Start != -1) {
                    cantidad_mounts++;
                }
            }
            if (cantidad_mounts == 0) {
                memset(&Sistema[i].Path, 0, 300);
                Sistema[i].letra = -1;
            }
            if (EMount == 1) {
                break;
            }
        }
    }
    if (EXMount == -1 && EMount == -1) {
        printf("ERROR: LA PARTICION BUSCADA NO SE ENCUENTRA MONTADA EN EL SISTEMA. \n");
    }
    Disco_Montado Aux;
    for (int i = 1; i < 26; i++) {
        for (int j = 0; j < 26 - i; j++) {
            if (((Sistema[j].letra > Sistema[j + 1].letra) && Sistema[j + 1].letra != -1) || (Sistema[j].letra == -1)) {
                Aux = Sistema[j + 1];
                Sistema[j + 1] = Sistema[j];
                Sistema[j] = Aux;
            }
        }
    }
    for (int i = 0; i < 26; i++) {
        if (Sistema[i].letra != -1) {
            Sistema[i].letra = 97 + i;
            for (int j = 0; j < 10; j++) {
                if (Sistema[i].Particiones[j].Start != -1) {
                    memset(&Sistema[i].Particiones[j].ID, 0, 5);
                    Sistema[i].Particiones[j].ID[0] = 'v';
                    Sistema[i].Particiones[j].ID[1] = 'd';
                    Sistema[i].Particiones[j].ID[2] = (char) Sistema[i].letra;
                    Sistema[i].Particiones[j].ID[3] = (char) 48 + j + 1;
                    Sistema[i].Particiones[j].ID[4] = '\0';
                }
            }
        }
    }
    Particion_Montada AOM;
    for (int i = 0; i < 26; i++) {
        if (Sistema[i].letra != -1) {
            for (int x = 1; x < 10; x++) {
                for (int y = 0; y < 10 - x; y++) {
                    if (Sistema[i].Particiones[y].Start == -1) {
                        AOM = Sistema[i].Particiones[y + 1];
                        Sistema[i].Particiones[y + 1] = Sistema[i].Particiones[y];
                        Sistema[i].Particiones[y] = AOM;
                    }
                }
            }
        }
    }
    for (int i = 0; i < 26; i++) {
        if (Sistema[i].letra != -1) {
            for (int j = 0; j < 10; j++) {
                if (Sistema[i].Particiones[j].Start != -1) {
                    memset(&Sistema[i].Particiones[j].ID, 0, 5);
                    Sistema[i].Particiones[j].ID[0] = 'v';
                    Sistema[i].Particiones[j].ID[1] = 'd';
                    Sistema[i].Particiones[j].ID[2] = (char) Sistema[i].letra;
                    Sistema[i].Particiones[j].ID[3] = (char) 48 + j + 1;
                    Sistema[i].Particiones[j].ID[4] = '\0';
                }
            }
        }
    }
}

int REP(char Destino[], char Disco[], int Tipo) {
    if (Validar_Ex(Disco) == 1) {
        Generar_Archivo(Destino);
        FILE *FRep;
        FRep = fopen(Destino, "w");
        if (FRep != NULL) {
            fclose(FRep);
            if (Tipo == 1) {
                return RepMBR(Destino, Disco);
            } else if (Tipo == 2) {
                return RepDISK(Destino, Disco);
            } else {
                printf("ERROR: EL VALOR DEL REPORTE NO ES VALIDO.\n");
                return -1;
            }
        } else {
            printf("ERROR: NO SE GENERO EL ARCHIVO DEL REPORTE. \n");
            return -1;
        }
    } else {
        printf("ERROR: NO SE PUDO ABRIR EL ARCHIVO DEL REPORTE. \n");
        return -1;
    }
}

int RepMBR(char Destino[], char Disco[]) {
    if (Validar_Ex(Destino) == 1) {
        remove(Destino);
    }
    int Rep_EX = 0;
    FILE *ARep;
    ARep = fopen(Destino, "w");
    if (ARep != NULL) {
        FILE *ADisco;
        ADisco = fopen(Disco, "rb+");
        if (ADisco != NULL) {
            char Contenido[1000] = {0};
            int NEBR = 0;
            MBR AMBR;
            fread(&AMBR, sizeof (AMBR), 1, ADisco);
            sprintf(Contenido, "digraph D {\nnode[shape=plaintext] rankdir=LR;\n  nodoUnico[label=< <table><tr><td colspan=\"3\">\"%s\"</td></tr>\n", Disco);
            fputs(Contenido, ARep);
            sprintf(Contenido, "<tr><td colspan=\"2\">Size (Bytes)</td><td> %i </td></tr>\n", AMBR.Size);
            fputs(Contenido, ARep);
            sprintf(Contenido, "<tr><td colspan=\"2\">ID Disco</td><td> %i </td></tr>\n", AMBR.ID);
            fputs(Contenido, ARep);
            sprintf(Contenido, "<tr><td colspan=\"2\">Fecha Creacion</td><td> %s </td></tr>\n", AMBR.FCreacion);
            fputs(Contenido, ARep);
            sprintf(Contenido, "<tr><td colspan=\"2\">Fit </td><td> %cf </td></tr>\n", AMBR.Fit);
            fputs(Contenido, ARep);
            for (int i = 0; i < 4; i++) {
                if (AMBR.Particiones[i].Start == -1) {
                    break;
                }
                sprintf(Contenido, "<tr><td colspan=\"3\">Particion  %i</td></tr>\n", i + 1);
                fputs(Contenido, ARep);
                sprintf(Contenido, "<tr><td colspan=\"2\">Estado </td><td> %c </td></tr>\n", AMBR.Particiones[i].Status);
                fputs(Contenido, ARep);
                sprintf(Contenido, "<tr><td colspan=\"2\">Tipo </td><td> %c </td></tr>\n", AMBR.Particiones[i].Type);
                fputs(Contenido, ARep);
                sprintf(Contenido, "<tr><td colspan=\"2\">Fit </td><td> %c </td></tr>\n", AMBR.Particiones[i].Fit);
                fputs(Contenido, ARep);
                sprintf(Contenido, "<tr><td colspan=\"2\">Inicio (Bytes)</td><td> %i </td></tr>\n", AMBR.Particiones[i].Start);
                fputs(Contenido, ARep);
                sprintf(Contenido, "<tr><td colspan=\"2\">Size (Bytes)</td><td> %i </td></tr>\n", AMBR.Particiones[i].Size);
                fputs(Contenido, ARep);
                sprintf(Contenido, "<tr><td colspan=\"2\">Nombre </td><td> %s </td></tr>\n", AMBR.Particiones[i].Name);
                fputs(Contenido, ARep);
                if (AMBR.Particiones[i].Type == 'e' && AMBR.Particiones[i].Start != -1) {
                    EBR AEBR;
                    fseek(ADisco, AMBR.Particiones[i].Start, SEEK_SET);
                    fread(&AEBR, sizeof (AEBR), 1, ADisco);
                    if (AEBR.Start != -1 || AEBR.Next != -1) {
                        sprintf(Contenido, "<tr><td></td><td colspan=\"2\">EBR #%i</td></tr>\n", NEBR + 1);
                        NEBR++;
                        fputs(Contenido, ARep);
                        if (!(AEBR.Status == 'I' || AEBR.Status == 'A')) {
                            sprintf(Contenido, "<tr><td width=\"20%\"></td><td>Estado</td><td> %c</td></tr>\n", 'X');
                        } else {
                            sprintf(Contenido, "<tr><td width=\"20%\"></td><td>Estado</td><td> %c</td></tr>\n", AEBR.Status);
                        }
                        fputs(Contenido, ARep);
                        if (!(AEBR.Fit == 'f' || AEBR.Fit == 'b' || AEBR.Fit == 'w')) {
                            sprintf(Contenido, "<tr><td width=\"20%\"></td><td>Fit</td><td> %c </td></tr>\n", 'X');
                        } else {
                            sprintf(Contenido, "<tr><td width=\"20%\"></td><td>Fit</td><td> %c </td></tr>\n", AEBR.Fit);
                        }
                        fputs(Contenido, ARep);
                        sprintf(Contenido, "<tr><td width=\"20%\"></td><td>Inicio (Bytes) </td><td> %i</td></tr>\n", AEBR.Start);
                        fputs(Contenido, ARep);
                        sprintf(Contenido, "<tr><td width=\"20%\"></td><td>Size (Bytes)</td><td> %i</td></tr>\n", AEBR.Size);
                        fputs(Contenido, ARep);
                        sprintf(Contenido, "<tr><td width=\"20%\"></td><td>Siguiente </td><td> %i</td></tr>\n", AEBR.Next);
                        fputs(Contenido, ARep);
                        sprintf(Contenido, "<tr><td width=\"20%\"></td><td>Nombre</td><td> %s</td></tr>\n", AEBR.Name);
                        fputs(Contenido, ARep);
                        while (AEBR.Next != -1 && AEBR.Start != 0) {
                            fseek(ADisco, AEBR.Next, SEEK_SET);
                            fread(&AEBR, sizeof (AEBR), 1, ADisco);
                            sprintf(Contenido, "<tr><td></td><td colspan=\"2\">EBR #%i</td></tr>\n", NEBR + 1);
                            NEBR++;
                            fputs(Contenido, ARep);
                            sprintf(Contenido, "<tr><td width=\"20%\"></td><td>Estado</td><td> %c</td></tr>\n", AEBR.Status);
                            fputs(Contenido, ARep);
                            sprintf(Contenido, "<tr><td width=\"20%\"></td><td>Fit</td><td> %c </td></tr>\n", AEBR.Fit);
                            fputs(Contenido, ARep);
                            sprintf(Contenido, "<tr><td width=\"20%\"></td><td>Inicio (Bytes)</td><td> %i</td></tr>\n", AEBR.Start);
                            fputs(Contenido, ARep);
                            sprintf(Contenido, "<tr><td width=\"20%\"></td><td>Size (Bytes)</td><td> %i</td></tr>\n", AEBR.Size);
                            fputs(Contenido, ARep);
                            sprintf(Contenido, "<tr><td width=\"20%\"></td><td>Siguiente </td><td> %i</td></tr>\n", AEBR.Next);
                            fputs(Contenido, ARep);
                            sprintf(Contenido, "<tr><td width=\"20%\"></td><td>Nombre Particion</td><td> %s</td></tr>\n", AEBR.Name);
                            fputs(Contenido, ARep);
                        }
                    }
                }
            }
            fclose(ADisco);
            Rep_EX = 1;
        } else {
            printf("ERROR: NO SE PUDO ABRIR EL DISCO \n");
        }
    } else {
        printf("ERROR: NO SE PUDO ABRIR EL ARCHIVO DEL REPORTE. \n");
        return -1;
    }
    if (Rep_EX == 1) {
        fputs("</table> >  ];}", ARep);
    }
    fclose(ARep);
    return Rep_EX;
}

int RepDISK(char Destino[], char Disco[]) {
    if (Validar_Ex(Destino) == 1) {
        remove(Destino);
    }
    int Rep_Ex = 0;
    int NEBR = 0;
    char Final[1000] = {0};
    FILE *ARep;
    ARep = fopen(Destino, "w");
    if (ARep != NULL) {
        char Contenido[1024] = {0};
        MBR AMBR;
        FILE *ArchD;
        ArchD = fopen(Disco, "rb+");
        if (ArchD != NULL) {
            fread(&AMBR, sizeof (AMBR), 1, ArchD);
            fseek(ArchD, 0, SEEK_SET);
            float TamMBR = AMBR.Size / 100;
            int SD = 140;
            int Pos = 0;
            sprintf(Contenido, "digraph D {\n  node [shape=plaintext] rankdir=LR;\n  nodoUnico[label=< <table><tr><td rowspan=\"2\">MBR<br/>Nombre: %s<br/>Creacion: %s<br/>Size: %i</td>\n", Disco, AMBR.FCreacion, AMBR.Size);
            fputs(Contenido, ARep);
            for (int i = 0; i < 4; i++) {
                Pos = i;
                if (AMBR.Particiones[i].Start == -1) {
                    break;
                }
                if (AMBR.Particiones[i].Start - SD > 0) {
                    sprintf(Contenido, "<td rowspan=\"2\">Libre<br/>%4.2f%</td>\n", (AMBR.Particiones[i].Start - SD) / TamMBR);
                    fputs(Contenido, ARep);
                }
                int Tot = 0;
                if (AMBR.Particiones[i].Type == 'e' && AMBR.Particiones[i].Start != -1) {
                    EBR AEBR;
                    fseek(ArchD, AMBR.Particiones[i].Start, SEEK_SET);
                    fread(&AEBR, sizeof (AEBR), 1, ArchD);
                    if (AEBR.Start == -1) {
                        sprintf(Final, "%s<td>EBR</td><td>Libre<br/>%4.2f%</td>\n", Final, (AEBR.Size / TamMBR));
                    } else {
                        sprintf(Final, "%s<td>EBR</td><td>Logica<br/>Nombre: %s<br/>%4.2f%</td>\n", Final, AEBR.Name, (AEBR.Size / TamMBR));
                    }
                    if (AEBR.Next - AEBR.Start - AEBR.Size > 0) {
                        Tot++;
                        sprintf(Final, "%s<td>Libre<br/>%4.2f%</td>\n", Final, ((AEBR.Next - AEBR.Start - AEBR.Size) / TamMBR));
                    }
                    NEBR++;
                    while (AEBR.Next != -1 && AEBR.Start != 0) {
                        fseek(ArchD, AEBR.Next, SEEK_SET);
                        fread(&AEBR, sizeof (AEBR), 1, ArchD);
                        sprintf(Final, "%s<td>EBR</td><td>Logica<br/>Nombre: %s<br/>%4.2f%</td>\n", Final, AEBR.Name, (AEBR.Size / TamMBR));
                        NEBR++;
                        if (AEBR.Next - AEBR.Start - AEBR.Size > 0) {
                            Tot++;
                            sprintf(Final, "%s<td>Libre<br/>%4.2f%</td>\n", Final, ((AEBR.Next - AEBR.Start - AEBR.Size) / TamMBR));
                        }
                    }
                    float posfinal = AMBR.Particiones[i].Start + AMBR.Particiones[i].Size - AEBR.Start - AEBR.Size;
                    if (posfinal > 0) {
                        sprintf(Final, "%s<td>Libre<br/>%4.2f%</td>  \n", Final, (posfinal / TamMBR));
                        sprintf(Contenido, "<td colspan=\"%d\">Extendida<br/>Nombre: %s<br/>%4.2f%</td> \n", NEBR * 2 + 1 + Tot, AMBR.Particiones[i].Name, AMBR.Particiones[i].Size / TamMBR);
                    } else {
                        sprintf(Contenido, "<td colspan=\"%d\">Extendida<br/>Nombre: %s<br/>%4.2f%</td> \n", NEBR * 2 + Tot, AMBR.Particiones[i].Name, AMBR.Particiones[i].Size / TamMBR);
                    }
                    fputs(Contenido, ARep);
                } else {
                    sprintf(Contenido, "<td rowspan=\"2\">Primaria<br/>Nombre: %s<br/>%4.2f%</td>  \n", AMBR.Particiones[i].Name, (AMBR.Particiones[i].Size / TamMBR));
                    fputs(Contenido, ARep);
                }
                SD = AMBR.Particiones[i].Start + AMBR.Particiones[i].Size;
            }
            if (Pos > 0) {
                if (Pos == 3) {
                    float posfinal = AMBR.Size - AMBR.Particiones[Pos].Start - AMBR.Particiones[Pos].Size;
                    if (posfinal > 140) {
                        sprintf(Contenido, "<td rowspan=\"2\">Libre<br/>%4.2f%</td>\n", (posfinal / TamMBR));
                        fputs(Contenido, ARep);
                    }
                } else {
                    float posfinal = AMBR.Size - AMBR.Particiones[Pos - 1].Start - AMBR.Particiones[Pos - 1].Size;
                    if (posfinal > 140) {
                        sprintf(Contenido, "<td rowspan=\"2\">Libre<br/>%4.2f%</td>\n", (posfinal / TamMBR));
                        fputs(Contenido, ARep);
                    }
                }
            } else {
                sprintf(Contenido, "<td rowspan=\"2\">Libre<br/>%4.2f%</td>\n", ((AMBR.Size - 140) / TamMBR));
            }
            fclose(ArchD);
            Rep_Ex = 1;
            printf("SE GENERO CON EXITO EL REPORTE. \n");
        } else {
            printf("ERROR: NO SE PUDO ABRIR EL DISCO \n");
            return false;
        }
    } else {
        printf("ERROR: NO SE PUDO ABRIR EL ARCHIVO DEL REPORTE. \n");
        return -1;
    }
    if (NEBR != 0 && Rep_Ex == 1) {
        fputs("</tr><tr>", ARep);
        fputs(Final, ARep);
    }
    if (Rep_Ex == 1) {
        fputs("</tr></table> >  ];}", ARep);
    }
    fclose(ARep);
    return Rep_Ex;
}

int EXEC(char path[]) {
    char Contenido[1000] = {0};
    if (Validar_Ex(path) == 1) {
        if (Validar_Extension(path, ".sh") == 0) {
            FILE *Script;
            Script = fopen(path, "r");
            if (Script != NULL) {
                while (!feof(Script)) {
                    fgets(Contenido, 1000, Script);
                    printf("%s", Contenido);
                    sprintf(Contenido, "%s \n", Contenido);
                    COMANDO(Contenido);
                }
            } else {
                printf("ERROR: NO SE PUDO ABRIR EL ARCHIVO. \n");
                return -1;
            }
        } else {
            printf("ERROR: NO SE RECONOCE LA TERMINACION DEL ARCHIVO. \n");
            return -1;
        }
    } else {
        printf("ERROR: EL ARCHIVO NO EXISTE. \n");
        return -1;
    }
    return 0;
}

void MKFS(char Path[], Particion_Montada Particion_Buscada, int Type, int FS) {
    if (Validar_Extension(Path, ".disk") == 0) {
        if (Validar_Ex(Path) == 1) {
            FILE *Disco_Actual;
            int I_Part = -1;
            int S_Part = -1;
            Disco_Actual = fopen(Path, "rb+");

            if (Disco_Actual != NULL) {

                if (Particion_Buscada.Type == 'p' || Particion_Buscada.Type == 'P') {
                    fseek(Disco_Actual, Particion_Buscada.Start, SEEK_SET);
                    I_Part = Particion_Buscada.Start;
                    S_Part = Particion_Buscada.Size;

                    if (FS == 3) {
                        if (Type == 1) {//Full
                            int SFisico = Particion_Buscada.Size;
                            int Val_Max = 1024 * 1024;
                            if (SFisico > Val_Max) {
                                char insert[1024 * 1024] = {0};
                                while (SFisico > Val_Max) {
                                    SFisico = SFisico - Val_Max;
                                    fwrite(insert, sizeof (char), Val_Max, Disco_Actual);
                                }
                            }
                            char Final_Cont[SFisico];
                            for (int i = 0; i < SFisico; i++) {
                                Final_Cont[i] = '\0';
                            }
                            fwrite(Final_Cont, sizeof (char), SFisico, Disco_Actual);
                            fseek(Disco_Actual, Particion_Buscada.Start, SEEK_SET);

                            int n = 1 + ((S_Part - sizeof (Super_Bloque) - 1) / (sizeof (Journal) + sizeof (Inodo) + 3 * sizeof (Content_Block) + 4));

                            //Creacion Superbloque
                            Super_Bloque Nuevo_SuperBloque;
                            Nuevo_SuperBloque = I_Super_Bloque(Nuevo_SuperBloque);

                            Nuevo_SuperBloque.FS_Type = FS;
                            Nuevo_SuperBloque.Inodes_Count = n;
                            Nuevo_SuperBloque.Blocks_Count = 3 * n;
                            Nuevo_SuperBloque.Free_Inodes_Count = n;
                            Nuevo_SuperBloque.Free_Blocks_Count = 3 * n;
                            time_t tiempoActual = time(NULL);
                            struct tm *fecha = localtime(&tiempoActual);
                            strftime(Nuevo_SuperBloque.MTime, sizeof (Nuevo_SuperBloque.MTime) - 1, "%d/%m/%y %H:%M", fecha);
                            memset(&Nuevo_SuperBloque.UMTime, 0, 16);
                            Nuevo_SuperBloque.Mnt_Count = 1;
                            Nuevo_SuperBloque.Magic = 61267;
                            Nuevo_SuperBloque.Inode_Size = sizeof (Inodo);
                            Nuevo_SuperBloque.Block_Size = sizeof (Content_Block);

                            I_Part = I_Part + sizeof (Super_Bloque) + n * sizeof (Journal);

                            Nuevo_SuperBloque.BM_Inode_Start = I_Part;
                            Nuevo_SuperBloque.BM_Block_Start = I_Part + n;
                            Nuevo_SuperBloque.Inode_Start = I_Part + n + 3 * n;
                            Nuevo_SuperBloque.Block_Start = I_Part + n + 3 * n + n * sizeof (Inodo);
                            Nuevo_SuperBloque.First_Ino = 2;
                            Nuevo_SuperBloque.First_Blo = 2;
                            fwrite(&Nuevo_SuperBloque, sizeof (Nuevo_SuperBloque), 1, Disco_Actual);

                            //Creacion Journal
                            Journal Nuevo_Journal;
                            Nuevo_Journal = I_Journal(Nuevo_Journal);
                            for (int i = 0; i < n; i++) {
                                fwrite(&Nuevo_Journal, sizeof (Journal), 1, Disco_Actual);
                            }

                            //Creacion BitMap
                            char vacio = '0';
                            char lleno = '1';
                            for (int i = 0; i < 4 * n; i++) {
                                if (i == n + 1) {
                                    lleno = '2';
                                    fwrite(&lleno, 1, 1, Disco_Actual);
                                } else if (i == 0 || i == 1 || i == n) {
                                    fwrite(&lleno, 1, 1, Disco_Actual);
                                } else {
                                    fwrite(&vacio, 1, 1, Disco_Actual);
                                }
                            }

                            //Creacion Tabla de Inodos.
                            Inodo Inodo_Nuevo;
                            Inodo Inodo_Auxiliar;
                            Inodo_Nuevo = I_Inodo(Inodo_Nuevo);
                            Inodo_Nuevo.I_UID = 1;
                            Inodo_Nuevo.I_GID = 1;
                            Inodo_Nuevo.I_Size = 0;
                            strftime(Inodo_Nuevo.I_CTime, sizeof (Inodo_Nuevo.I_CTime) - 1, "%d/%m/%y %H:%M", fecha);
                            strftime(Inodo_Nuevo.I_ATime, sizeof (Inodo_Nuevo.I_ATime) - 1, "%d/%m/%y %H:%M", fecha);
                            strftime(Inodo_Nuevo.I_MTime, sizeof (Inodo_Nuevo.I_MTime) - 1, "%d/%m/%y %H:%M", fecha);
                            Inodo_Nuevo.I_Block[0] = 0;

                            for (int i = 1; i < 15; i++) {
                                Inodo_Nuevo.I_Block[i] = -1;
                            }

                            Inodo_Nuevo.I_Type = '0';
                            Inodo_Nuevo.I_Perm = 664;

                            fseek(Disco_Actual, ST_Inodo(0, Nuevo_SuperBloque.Inode_Start), SEEK_SET);
                            fwrite(&Inodo_Nuevo, sizeof (Inodo_Nuevo), 1, Disco_Actual);
                            Content_Block nuevo_bloque;
                            File_Block Archivo_Nuevo;
                            nuevo_bloque = I_Content_Block(nuevo_bloque);
                            nuevo_bloque.B_Content[0].B_Inodo = 0;
                            strcpy(nuevo_bloque.B_Content[0].B_Name, ".");
                            nuevo_bloque.B_Content[1].B_Inodo = 0;
                            strcpy(nuevo_bloque.B_Content[1].B_Name, "..");
                            nuevo_bloque.B_Content[2].B_Inodo = 1;
                            strcpy(nuevo_bloque.B_Content[2].B_Name, "users.txt");
                            nuevo_bloque.B_Content[3].B_Inodo = -1;
                            fseek(Disco_Actual, ST_Bloque(0, Nuevo_SuperBloque.Block_Start), SEEK_SET);
                            fwrite(&nuevo_bloque, sizeof (nuevo_bloque), 1, Disco_Actual);
                            fseek(Disco_Actual, ST_Inodo(1, Nuevo_SuperBloque.Inode_Start), SEEK_SET);

                            char primer_usuarios[] = "1,G,root\n1,U,root,root,123\n";
                            Inodo_Auxiliar.I_UID = 1;
                            Inodo_Auxiliar.I_GID = 1;
                            Inodo_Auxiliar.I_Size = strlen(primer_usuarios);
                            strftime(Inodo_Auxiliar.I_CTime, sizeof (Inodo_Auxiliar.I_CTime) - 1, "%d/%m/%y %H:%M", fecha);
                            strftime(Inodo_Auxiliar.I_ATime, sizeof (Inodo_Auxiliar.I_ATime) - 1, "%d/%m/%y %H:%M", fecha);
                            strftime(Inodo_Auxiliar.I_MTime, sizeof (Inodo_Auxiliar.I_MTime) - 1, "%d/%m/%y %H:%M", fecha);
                            Inodo_Auxiliar.I_Block[0] = 1;
                            for (int i = 1; i < 15; i++) {
                                Inodo_Auxiliar.I_Block[i] = -1;
                            }

                            Inodo_Auxiliar.I_Type = '1';
                            Inodo_Auxiliar.I_Perm = 664;
                            fwrite(&Inodo_Auxiliar, sizeof (Inodo), 1, Disco_Actual);

                            fseek(Disco_Actual, ST_Bloque(1, Nuevo_SuperBloque.Block_Start), SEEK_SET);
                            Archivo_Nuevo = initFileBlock(Archivo_Nuevo);
                            strcpy(Archivo_Nuevo.B_Content, primer_usuarios);
                            fwrite(&Archivo_Nuevo, sizeof (File_Block), 1, Disco_Actual);
                            fclose(Disco_Actual);

                            printf("SE FORMATEO CON EXITO LA PARTICION: (n '%i'): INODOS '%i' , BLOQUES '%i', JOURNAL '%i' \n", n, n, 3 * n, n);

                            Disco_Actual = fopen(Path, "rb+");
                            Journal NJournal;
                            NJournal.Journal_Propietario = 1;
                            strftime(NJournal.Journal_Fecha, sizeof (NJournal.Journal_Fecha) - 1, "%d/%m/%y %H:%M", fecha);
                            strcpy(NJournal.Journal_Nombre, "/users.txt");
                            NJournal.Journal_Permisos = 664;
                            NJournal.Journal_Tipo = '1';
                            char PathJournal[] = "0@/home/firstUsers.txt@0";
                            strcpy(NJournal.Journal_Contenido, PathJournal);
                            NJournal.Journal_Tipo_Operacion = 'a';

                            int start = Particion_Buscada.Start + sizeof (Super_Bloque);
                            fseek(Disco_Actual, start, SEEK_SET);
                            fwrite(&NJournal, sizeof (Journal), 1, Disco_Actual);
                            fclose(Disco_Actual);
                        } else {//Fast
                            fseek(Disco_Actual, Particion_Buscada.Start, SEEK_SET);
                            int n = -1;
                            n = 1 + ((S_Part - sizeof (Super_Bloque) - 1) / (sizeof (Journal) + sizeof (Inodo) + 3 * sizeof (Content_Block) + 4));
                            Super_Bloque Nuevo_SuperBloque;
                            Nuevo_SuperBloque = I_Super_Bloque(Nuevo_SuperBloque);

                            Nuevo_SuperBloque.FS_Type = FS;
                            Nuevo_SuperBloque.Inodes_Count = n;
                            Nuevo_SuperBloque.Blocks_Count = 3 * n;
                            Nuevo_SuperBloque.Free_Inodes_Count = n;
                            Nuevo_SuperBloque.Free_Blocks_Count = 3 * n;
                            time_t tiempoActual = time(NULL);
                            struct tm *fecha = localtime(&tiempoActual);
                            strftime(Nuevo_SuperBloque.MTime, sizeof (Nuevo_SuperBloque.MTime) - 1, "%d/%m/%y %H:%M", fecha);
                            memset(&Nuevo_SuperBloque.UMTime, 0, 16);
                            Nuevo_SuperBloque.Mnt_Count = 1;
                            Nuevo_SuperBloque.Magic = 61267;
                            Nuevo_SuperBloque.Inode_Size = sizeof (Inodo);
                            Nuevo_SuperBloque.Block_Size = sizeof (Content_Block);

                            I_Part = I_Part + sizeof (Super_Bloque) + n * sizeof (Journal);

                            Nuevo_SuperBloque.BM_Inode_Start = I_Part;
                            Nuevo_SuperBloque.BM_Block_Start = I_Part + n;
                            Nuevo_SuperBloque.Inode_Start = I_Part + n + 3 * n;
                            Nuevo_SuperBloque.Block_Start = I_Part + n + 3 * n + n * sizeof (Inodo);

                            Nuevo_SuperBloque.First_Ino = 2;
                            Nuevo_SuperBloque.First_Blo = 2;
                            fwrite(&Nuevo_SuperBloque, sizeof (Nuevo_SuperBloque), 1, Disco_Actual);

                            Journal nueva_entrada;
                            nueva_entrada = I_Journal(nueva_entrada);
                            for (int i = 0; i < n; i++) {
                                fwrite(&nueva_entrada, sizeof (Journal), 1, Disco_Actual);
                            }

                            char vacio = '0';
                            char lleno = '1';
                            for (int i = 0; i < 4 * n; i++) {
                                if (i == n + 1) {
                                    lleno = '2';
                                    fwrite(&lleno, 1, 1, Disco_Actual);
                                } else if (i == 0 || i == 1 || i == n) {
                                    fwrite(&lleno, 1, 1, Disco_Actual);
                                } else {
                                    fwrite(&vacio, 1, 1, Disco_Actual);
                                }
                            }

                            Inodo nuevo_inodo;
                            Inodo nuevo_inodo_2;
                            nuevo_inodo = I_Inodo(nuevo_inodo);
                            nuevo_inodo.I_UID = 1;
                            nuevo_inodo.I_GID = 1;
                            nuevo_inodo.I_Size = 0;
                            strftime(nuevo_inodo.I_CTime, sizeof (nuevo_inodo.I_CTime) - 1, "%d/%m/%y %H:%M", fecha);
                            strftime(nuevo_inodo.I_ATime, sizeof (nuevo_inodo.I_ATime) - 1, "%d/%m/%y %H:%M", fecha);
                            strftime(nuevo_inodo.I_MTime, sizeof (nuevo_inodo.I_MTime) - 1, "%d/%m/%y %H:%M", fecha);
                            nuevo_inodo.I_Block[0] = 0;
                            for (int i = 1; i < 15; i++) {
                                nuevo_inodo.I_Block[i] = -1;
                            }
                            nuevo_inodo.I_Type = '0';
                            nuevo_inodo.I_Perm = 664;
                            fseek(Disco_Actual, ST_Inodo(0, Nuevo_SuperBloque.Inode_Start), SEEK_SET);
                            fwrite(&nuevo_inodo, sizeof (nuevo_inodo), 1, Disco_Actual);

                            Content_Block nuevo_bloque;
                            File_Block nuevo_archivo;
                            nuevo_bloque = I_Content_Block(nuevo_bloque);
                            nuevo_bloque.B_Content[0].B_Inodo = 0;
                            strcpy(nuevo_bloque.B_Content[0].B_Name, ".");
                            nuevo_bloque.B_Content[1].B_Inodo = 0;
                            strcpy(nuevo_bloque.B_Content[1].B_Name, "..");
                            nuevo_bloque.B_Content[2].B_Inodo = 1;
                            strcpy(nuevo_bloque.B_Content[2].B_Name, "users.txt");
                            nuevo_bloque.B_Content[3].B_Inodo = -1;
                            fseek(Disco_Actual, ST_Bloque(0, Nuevo_SuperBloque.Block_Start), SEEK_SET);
                            fwrite(&nuevo_bloque, sizeof (nuevo_bloque), 1, Disco_Actual);

                            fseek(Disco_Actual, ST_Inodo(1, Nuevo_SuperBloque.Inode_Start), SEEK_SET);
                            char primer_usuarios[] = "1,G,root\n1,U,root,root,123\n";
                            nuevo_inodo_2.I_UID = 1;
                            nuevo_inodo_2.I_GID = 1;
                            nuevo_inodo_2.I_Size = strlen(primer_usuarios);
                            strftime(nuevo_inodo_2.I_CTime, sizeof (nuevo_inodo_2.I_CTime) - 1, "%d/%m/%y %H:%M", fecha);
                            strftime(nuevo_inodo_2.I_ATime, sizeof (nuevo_inodo_2.I_ATime) - 1, "%d/%m/%y %H:%M", fecha);
                            strftime(nuevo_inodo_2.I_MTime, sizeof (nuevo_inodo_2.I_MTime) - 1, "%d/%m/%y %H:%M", fecha);
                            nuevo_inodo_2.I_Block[0] = 1;
                            for (int i = 1; i < 15; i++) {
                                nuevo_inodo_2.I_Block[i] = -1;
                            }
                            nuevo_inodo_2.I_Type = '1';
                            nuevo_inodo_2.I_Perm = 664;
                            fwrite(&nuevo_inodo_2, sizeof (Inodo), 1, Disco_Actual);
                            fseek(Disco_Actual, ST_Bloque(1, Nuevo_SuperBloque.Block_Start), SEEK_SET);
                            nuevo_archivo = initFileBlock(nuevo_archivo);
                            strcpy(nuevo_archivo.B_Content, primer_usuarios);
                            fwrite(&nuevo_archivo, sizeof (File_Block), 1, Disco_Actual);
                            fclose(Disco_Actual);

                            printf("SE FORMATEO CON EXITO LA PARTICION: (n '%i'): INODOS '%i' , BLOQUES '%i', JOURNAL '%i' \n", n, n, 3 * n, n);

                            Disco_Actual = fopen(Path, "rb+");
                            Journal entrada;
                            entrada.Journal_Propietario = 1;
                            strftime(entrada.Journal_Fecha, sizeof (entrada.Journal_Fecha) - 1, "%d/%m/%y %H:%M", fecha);
                            strcpy(entrada.Journal_Nombre, "/users.txt");
                            entrada.Journal_Permisos = 664;
                            entrada.Journal_Tipo = '1';

                            char pathAuxFile2[] = "0@/home/MIA/firstUsers.txt@0";

                            strcpy(entrada.Journal_Contenido, pathAuxFile2);
                            entrada.Journal_Tipo_Operacion = 'a';

                            int start = Particion_Buscada.Start + sizeof (Super_Bloque);
                            fseek(Disco_Actual, start, SEEK_SET);
                            fwrite(&entrada, sizeof (Journal), 1, Disco_Actual);
                            fclose(Disco_Actual);
                        }
                    } else {
                        printf("ERROR: EL SISTEMA SOLO TRABAJA EN FORMATO EXT3. \n");
                    }
                } else {
                    printf("ERROR: EL TIPO DE LA PARTICION SELECCIONADA NO ES VALIDO. \n");
                }
            } else {
                printf("ERROR: NO SE PUEDE INGRESAR AL DISCO ACTUAL. \n");
            }
        } else {
            printf("ERROR: EL DISCO SELECCIONADO NO EXISTE. \n");
        }
    } else {
        printf("ERROR: EL DISCO SELECCIONADO NO ES VALIDO.\n");
    }
}

void LOGIN(char USR[], char PWD[], char Ruta[], Particion_Montada Particion, FILE *Disco) {
    Entidad Grupos[200];
    Entidad Usuarios[200];
    int nGrupos = 0;
    int nUsuarios = 0;
    Super_Bloque Aux;
    Aux = G_Super_Bloque(Particion, Disco);
    if (Aux.Magic == 61267) {
        Archivo Arch_AC;
        Arch_AC = G_Arch("users.txt", Particion, Aux.Inode_Start, Disco);
        if (Arch_AC.Estado != -1) {
            if (Disco != NULL) {
                int Block_Start = Aux.BM_Block_Start;
                int Block_Count = Aux.Blocks_Count;
                char BM_Bloque[270000] = {0};
                char Bit_Auxiliar;
                fseek(Disco, Block_Start, SEEK_SET);

                for (int i = 0; i < Block_Count; i++) {
                    fread(&Bit_Auxiliar, 1, 1, Disco);
                    BM_Bloque[i] = Bit_Auxiliar;
                    BM_Bloque[i + 1] = '\0';
                }

                Block_Start = Aux.BM_Inode_Start;
                Block_Count = Aux.Inodes_Count;
                fseek(Disco, Block_Start, SEEK_SET);
                char BM_Inode[270000] = {0};

                for (int i = 0; i < Block_Count; i++) {
                    fread(&Bit_Auxiliar, 1, 1, Disco);
                    BM_Inode[i] = Bit_Auxiliar;
                    BM_Inode[i + 1] = '\0';
                }
                USR_Ac.Start_INodes = Aux.Inode_Start;
                USR_Ac.Start_Blocks = Aux.Block_Start;
                strcpy(USR_Ac.BM_Block, BM_Bloque);
                strcpy(USR_Ac.BM_INode, BM_Inode);

                char AX[9999] = {0};
                char AX2[9999] = {0};
                char C_Arch[999];
                char Grupo[999] = {0};
                char com[4][1000];
                strcpy(C_Arch, Arch_AC.Contenido);
                int i = 0;
                int length = 0;
                int comas = 0;
                int comillas = 0;
                int status = 0;
                int I_US;

                while (C_Arch[i] != NULL) {
                    char aux_char[] = {C_Arch[i], '\0'};
                    if (C_Arch[i] == '\n') {
                        strcpy(AX2, "");
                        comillas = 0;
                        for (int j = 0; j < length; j++) {
                            char aux_char_2[] = {AX[j], '\0'};
                            if (AX[j] == ',') {
                                strcpy(com[comillas], AX2);
                                comillas++;
                                strcpy(AX2, "");
                            } else {
                                strcat(AX2, aux_char_2);
                            }
                        }
                        if (comas == 2) {
                            Grupos[nGrupos].N_Grupo = atoi(com[0]);
                            strcpy(Grupos[nGrupos].Name, AX2);
                            if (status == 1) {
                                if (strcmp(Grupo, AX2) == 0) {
                                    USR_Ac.ID_Grupo = Grupos[nGrupos].N_Grupo;
                                    strcpy(USR_Ac.ID_Particion, Particion.Name);
                                    strcpy(USR_Ac.Path_Disco, Ruta);
                                    strcpy(USR_Ac.Name, USR);
                                    USR_Ac.P_Usuario = Particion;
                                    printf("SE INGRESO CON EXITO. \n USUARIO ACTUAL: %s", USR);
                                    strcpy(USR_Ac.BM_Block, BM_Bloque);
                                    strcpy(USR_Ac.BM_INode, BM_Inode);
                                }
                            }
                            nGrupos++;
                        } else if (status == 0) {
                            I_US = atoi(com[0]);
                            Usuarios[nUsuarios].N_Grupo = atoi(com[0]);
                            strcpy(Usuarios[nUsuarios].Group, com[2]);
                            strcpy(Usuarios[nUsuarios].Name, com[3]);
                            nUsuarios++;
                            strcpy(Grupo, com[2]);
                            if (strcmp(USR, com[3]) == 0 && strcmp(PWD, AX2) == 0) {
                                status = 1;
                                USR_Ac.ID_User = I_US;
                                for (int j = 0; j < nGrupos; j++) {
                                    if (strcmp(Grupo, Grupos[j].Name) == 0) {
                                        USR_Ac.ID_Grupo = Grupos[j].N_Grupo;
                                        strcpy(USR_Ac.ID_Particion, Particion.Name);
                                        strcpy(USR_Ac.Path_Disco, Ruta);
                                        strcpy(USR_Ac.Name, USR);
                                        USR_Ac.P_Usuario = Particion;
                                        printf("SE INGRESO CON EXITO. \n USUARIO ACTUAL: %s", USR);
                                    }
                                }
                            }
                        }
                        strcpy(AX, "");
                        comas = 0;
                        length = 0;
                    } else {
                        strcat(AX, aux_char);
                        length++;
                        if (C_Arch[i] == ',') {
                            comas++;
                        }
                    }
                    i++;
                }
            }
        }
        for (int i = 0; i < nUsuarios; i++) {
            Ent_Globales[i] = Usuarios[i];
        }
        NEnt_Globales = nUsuarios;
    } else {
        printf("ERROR: EL SISTEMA DE ARCHIVOS NO PUEDE SER ACCEDIDO. \n");
    }
}

int Validar_Ex(char *Path) {
    int Res = open(Path, 0);
    if (Res < 0) {
        return 0;
    }
    //close(Res);
    return 1;
}

int Generar_Archivo(char Ruta[]) {
    int i = 0;
    int Estado = 0;
    char Aux[500] = {0};

    if (Ruta[0] != 47) {
        printf("ERROR: HAY INFORMACION INVALIDA EN EL PATH. \n");
        return 0;
    }

    while (Ruta[i] != NULL) {
        char AXC[] = {Ruta[i], '\0'};
        strcat(Aux, AXC);

        if (Ruta[i] == 47) {
            Estado = 0;
            if (Validar_Ex(Aux) == 0) {
                Estado = mkdir(Aux, S_IRWXU | S_IRWXG | S_IRWXO);
            }

            if (Estado == -1) {
                printf("ERROR: NO SE PUDO CREAR EL ARCHIVO.\n");
            }
        }
        i++;
    }
    return 1;
}

int Validar_Extension(char Entrada[], char Valor[]) {
    int Tam = strlen(Valor);
    char Aux[Tam + 1];

    int Pos_Ac = 0;
    for (int i = strlen(Entrada) - Tam; i < strlen(Entrada); i++) {
        Aux[Pos_Ac] = Entrada[i];
        Pos_Ac++;
    }
    Aux[Pos_Ac] = '\0';


    if (strcmp(Aux, Valor) == 0) {
        return 0;
    }
    return -1;
}

int ISist() {
    for (int i = 0; i < 26; i++) {
        Sistema[i].letra = -1;
        memset(&Sistema[i].Path, 0, 300);
        for (int j = 0; j < 10; j++) {
            Sistema[i].Particiones[j].Start = -1;
            Sistema[i].Particiones[j].Size = -1;

            Sistema[i].Particiones[j].Status = 'I';
            Sistema[i].Particiones[j].Type = 'p';
            Sistema[i].Particiones[j].Fit = 'f';

            memset(&Sistema[i].Particiones[j].ID, 0, 5);
            memset(&Sistema[i].Particiones[j].Name, 0, 16);
        }
    }
    return 0;
}

void Cambiar_Formato(char Cadena[]) {
    int Comilla_Activa = 0;
    int Pos_Ac = 0;
    while (Cadena[Pos_Ac] != NULL) {
        if ((Cadena[Pos_Ac] == 34 || Cadena[Pos_Ac] == 47) && Comilla_Activa == 0) {
            Comilla_Activa = 1;
        } else if ((Cadena[Pos_Ac] == 34 || Cadena[Pos_Ac] == 32) && Comilla_Activa == 1) {
            Comilla_Activa = 0;
        }

        if ((Cadena[Pos_Ac] >= 65 && Cadena[Pos_Ac] <= 90) && Comilla_Activa == 0) {
            Cadena[Pos_Ac] = Cadena[Pos_Ac] + 32;
        }
        Pos_Ac++;
    }
}