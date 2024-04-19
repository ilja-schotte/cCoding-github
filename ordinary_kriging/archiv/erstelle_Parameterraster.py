#!/opt/WetterDE/env/bin/python3


import os, sys

Configverzeichnis = '/opt/WetterDE/config'
sys.path.append(Configverzeichnis)
import konfiguration

Modulverzeichnis = konfiguration.Module.module_dir
sys.path.append(Modulverzeichnis)

import request_Parameterwerte
import create_Parameterraster
import write_Parameterraster

import check_DB
import time
import numpy as np

########################################################################
########################################################################
ftp_server = konfiguration.ftp_server.name
ftp_usr = konfiguration.ftp_server.usr
ftp_pwd = konfiguration.ftp_server.pwd
down_dir = konfiguration.reports_download.quelle
save_dir = konfiguration.reports_download.ziel
db_name = konfiguration.Datenbank.name
db_host = konfiguration.Datenbank.host
db_usr = konfiguration.Datenbank.usr
db_pwd = konfiguration.Datenbank.pwd
db_port = konfiguration.Datenbank.port

########################################################################

tmp_dir = konfiguration.rasterImages.verarbeitung_Rasterbilder		# Verarbeitungsordner
save_dir = konfiguration.rasterImages.speicherort_Rasterbilder		# Speicherort der fertigen Raster
shapefile_germany = konfiguration.rasterImages.Deutschland_Shapefile	# Shapefile Deutschland

########################################################################
########################################################################


t = time.localtime()
start_time = time.strftime("%d.%m.%Y %H:%M:%S", t)
print("=================================================================")
print("Abruf gestartet um:",start_time)


# Für welchen Parameter soll eine Karte erstellt werden?:
parameter = "Temperatur2m"

# Stelle eine Verbindung zur Datenbank her:
connection = check_DB.verbinde_zu_Datenbank(db_host, db_usr, db_pwd, db_name, db_port)


# Abrufen der Termine, für die eine Karte erstellt werden soll:
try:
    data = request_Parameterwerte.loadDataFromDB(connection, parameter)	# data: GroupBy-Object
except:
    sys.exit(1)


# Erstelle aus den Daten zuerst interpolierte Arrays:
if data:
    try:
        interpolated_arrays = create_Parameterraster.createInterpolatedArray(data) # dict
    except:
        print(sys.exc_info())
        sys.exit(1)
else:
    print("Keine neuen Daten für Rastererstellung.")
    sys.exit(0)




# Dann wird zwischen den stündlichen Arrays auf halbstündliche interpoliert:
if interpolated_arrays:
   try:
       temp_interpolated_arrays = create_Parameterraster.createTemporallyInterpolatedArrays(interpolated_arrays)
   except:
       print(sys.exc_info())
       sys.exit(1)
else:
    print("Keine Arrays für zeitliche Interpolation vorhanden.")
    sys.exit(0)


# Erstelle aus den interpolierten Arrays Rasterbilder:
if temp_interpolated_arrays:
    try:
        images_to_write_in_db = create_Parameterraster.createRasterImage(temp_interpolated_arrays, parameter, shapefile_germany, tmp_dir, save_dir)
    except:
        print(sys.exc_info())
        sys.exit(1)
else:
    print("Keine Arrays für Rasterbilderstellung produziert.")
    sys.exit(0)


# Schreibe die Informationen der erstellten Rasterbilder in die Datenbank::
if not images_to_write_in_db.empty:
    try:
        write_Parameterraster.writeImagesToDB(images_to_write_in_db, connection)
    except:
        print(sys.exc_info())
        sys.exit(1)
else:
    print("Keine Rasterbilder erstellt.")
    sys.exit(0)
    
# Am Ende immer Aufräumen!:
write_Parameterraster.aufraeumen(tmp_dir, save_dir, connection)

