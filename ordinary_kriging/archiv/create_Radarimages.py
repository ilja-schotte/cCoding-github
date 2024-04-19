#!/opt/WetterDE/env/bin/python3


import os, sys
import traceback
import numpy as np
import tarfile
import re
import pandas as pd
import time
import geojson as gj
import json
import cartopy.io.shapereader as shpreader
import cartopy.crs as ccrs
import matplotlib.pyplot as plt
import sqlalchemy
from sqlalchemy import text
from jsmin import jsmin
from shapely.geometry import Polygon, mapping, MultiPolygon, Point
from shapely.ops import unary_union
from PIL import Image, ImageOps
from matplotlib.colors import LinearSegmentedColormap
from datetime import datetime


# Konfiguration importieren:
Configverzeichnis = '/opt/WetterDE/config'
sys.path.append(Configverzeichnis)
import konfiguration

# Eigene Module importieren:
Modulverzeichnis = konfiguration.Module.module_dir
sys.path.append(Modulverzeichnis)
import check_DB
#######################################################################
db_name = konfiguration.Datenbank.name			# Datenbankname
db_host = konfiguration.Datenbank.host			# Host
db_usr = konfiguration.Datenbank.usr			# Benutzer
db_pwd = konfiguration.Datenbank.pwd			# PW
db_port = konfiguration.Datenbank.port			# port
########################################################################


class Radarimage():


    rows = ""
    columns = ""
    product = ""
    filesize = ""
    offsetsize = ""
    binarysize = ""
    radararchive = ""
    mask_name = ""


    def __init__(self, R, lam, phi, date, str_date, tmp_dir, save_dir, to_produce, product, shapefile, fileformat):

        self.product = product
        self.to_produce = to_produce
        self.date = date
        self.str_date = str_date
        self.tmp_dir = tmp_dir
        self.save_dir = save_dir
        self.shapefile = shapefile
        self.fileformat = fileformat

        self.earthRadius = float(R)
        self.lambda_0 = float(lam)*(np.pi/180)
        self.phi_0 = float(phi)*(np.pi/180)

        self.coords_lat_NW = 55.86584289
        self.coords_lon_NW = 1.435612143
        # ------------------------------
        self.coords_lat_NE = 55.84848692
        self.coords_lon_NE = 18.76728172
        # ------------------------------
        self.coords_lat_SW = 45.69587048
        self.coords_lon_SW = 3.551921296
        # ------------------------------
        self.coords_lat_SE = 45.68358331
        self.coords_lon_SE = 16.60186543




    def show_Information(self):

        # Zeigt sämtliche Informationen zum Radarbild auf stdout an:

        print("Produkt:", self.product)
        print("Endprodukt:", self.to_produce)
        print("Termin:",self.date)
        print("Termin (String):",self.str_date)
        print("Radararchive:", self.radararchive)
        print("Dateiname:", self.filename)
        print("Zeilen:",self.rows)
        print("Spalten:",self.columns)
        print("Groesse:",self.filesize)
        print("Größe Binärteil:", self.binarysize)
        print("Größe ASCII-Teile (offset):", self.offsetsize)
        print("Shapedatei:", self.shapefile)
        print("Erdradius:",self.earthRadius)
        print("Lambda 0:",self.lambda_0)
        print("Phi 0:",self.phi_0)
        print("tmp_dir:",self.tmp_dir)
        print("save_dir:",self.save_dir)
        print("mask_name:", self.mask_name)




    def unzip_and_read_radarfile(self, radararchive):
        
        # Entpackt und öffnet die erste Datei im Archiv, wenn es sich um eine Datei mit der Endung "_000" handelt.
        # Die Datei mit der Endung "_000" ist das aktuelle Radarbild zum Termin, alle weiteren sind Prognosedate.
        # Dabei entsprechen die Ziffern den Zeitraum der Prognose in Minuten.

        self.radararchive = radararchive

        try:
            # Öffnen des Radardatenarchivs:
            with tarfile.open(radararchive, "r:bz2") as RTF: 		# Radar.Tar.File
               
                # Find den Namen der ersten Datei im Archiv:
                filename = RTF.getnames()[0]

                # Prüfe ob Datei auf "_000" endet:
                if filename[-4:] == "_000":

                    # Extrahiere die Datei:
                    radarfile = RTF.extractfile(filename)
                    
                    # Einlesen der Daten:
                    content = radarfile.read()

                    return content

                


        except Exception as e:
            print("============================================================") 
            print("Fehler beim entpacken der Datei",self.str_date,"!")
            print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
            print("============================================================")
            print(e)
            print(sys.exc_info())
            print(traceback.format_exc())
            sys.exit(1)





    def get_fileproperties(self, content):

        """ Teilt den Dateiinhalt, anhand des x03 Zeichens in 2 Teile
            und extrahiert den Meldungkopf vom Binärteil

            Anschließend werden:
            - Produktkennung
            - Datum (DDMMYYhhmm)
            - Tag
	    - Zeit
	    - Anzahl Zeilen
	    - Anzahl Spalten
	    bestimmt und in einer Series ausgegeben.

            Input: Datei, die im binären Modus geöfnet und mit read(eingelesen wurde.)
        """

        # Der binäre Dateiinhalt wird an der Kennung '\x03' geteilt und
        # der erste Teil als ASCII-Header der Datei extrahiert und anschließend in einen utf-8 String dekodiert:
        try:

            sHeader = content.split(b'\x03')[0].decode("utf-8")

        except Exception as e:
            print("============================================================")
            print("Fehler bei der Trennung des Kopfteils vom Binärteil !!!")
            print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
            print("============================================================")
            print(e)
            print(sys.exc_info())
            print(traceback.format_exc())
            sys.exit(1)

        try:
            #############################################################################################################
            # Anzahl Zeilen und Spalten
            #############################################################################################################

            # Finde die Position "GP" und lade die folgenden 9 Zeichen (Auflösung: z.b. 1100x1200 = 9 Zeichen):
            Kennung = "GP"

            # Kompiliere den entsprechenden regulären Ausdruck:
            regex = re.compile(Kennung)

            # Finde die Position im Header, wo die Kennung "GP" beginnt und extrahiere die 9 Zeichen nach dieser Kennung:
            for m in regex.finditer(sHeader):
                substr = sHeader[m.start()+2 : m.start()+2+9]


            self.rows = int(substr.split('x')[0])
            self.columns = int(substr.split('x')[1])
    

            #############################################################################################################
            # Finde Produktkennung:
            #############################################################################################################

            self.product = sHeader[0:2]

            #############################################################################################################
            # Eine Datei hat die Größe von 2640091 Bytes:
            # Die Datei besteht aus einem ASCII-Teil und einem binär-Teil

            # Die Größe des binär-Teils ermittelt sich aus: 1100 x 1200 = (x*y) = Spalten x Zeilen in Bytes = 1320000 Bytes.
            # Da jeder Wert aus 2 Bytes besteht (little endian) muss dieser Wert verdoppelt werden (x2)
            # Und somit ist die Größe des Binärteils: 1100 bytes x 1200 bytes x 2 bytes = 2640000 bytes

            # Die Größe des ASCII-Teils (offset) ermittelt sich aus:
            # Dateigröße (2640091 Bytes) minus der Größe des binär-Teils
            # Folglich ist die Größe des ASCII-Teils (offset): 2640091 bytes - (1100 bytes x 1200 bytes x 2 bytes) = 91 bytes
            #############################################################################################################

            # Berechnen der Größe der Datei (Länge des Dateiinhalts):
            self.filesize = int(len(content))

            # Berechnen der Größe des Offsets (s.o.):
            self.offsetsize = int(self.filesize - (self.rows * self.columns)*2)

            # Berechnung der Größe des Binärteils:
            self.binarysize = int(self.filesize - self.offsetsize)


        except Exception as e:
            print("============================================================")
            print("Fehler beim Ermitteln der Zeilen und Spalten der Datei self.!!!")
            print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
            print("============================================================")
            print(e)
            print(sys.exc_info())
            print(traceback.format_exc())
            sys.exit(1)





    def extract_binary(self, content):
        """ Trennt von der Binärdatei den Binärteil ab.
            Input: 	Datei, die im Binärmodus geöffnet und mit read() eingelesen wurde.
           Output:	Binäre Sequenz	
        """

        try:
            # Abschneiden des ASCII-Teils (offset) vom Binärteil:
            binary = bytearray(content)[self.offsetsize:]

            return binary

        except Exception as e:
            print("============================================================")
            print("Fehler bei der Trennung des Kopfteils vom Binärteil !!!") 
            print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
            print("============================================================")     
            print(e)
            print(sys.exc_info())
            print(traceback.format_exc()) 
            sys.exit(1)





    def create_RVP6_array(self, binary):


        """ Ließt den Binärteil und wandelt diesen in unsigned 16 Bit Integers (0-4065) um.
            Input:	binary = Binäre Sequenz.
            Output:	Array mit Werten zwischen 0 bis 4095	
            
            RVP6 ... Rohwert der Reflektivität
        """
        try:
            # Umwandlung des kompletten binary-arrays in ein 1D-bits Array [1 1 0 ... 1 0 0 1]:
            bits_array = np.unpackbits(binary)

            # Ermittle die Anzahl der Werte im bits_array:
            # Anzahl der bits im array, geteilt durch 16 bit (da 2 Byte little endian) = 1320000 Werte => 1200 Zeilen x 1100 Spalten
            div = int(len(bits_array)/16) 	# sollte 1320000 sein

            RVP6_list = []
            n = 12		# Anzahl der Bits pro RVP6 Wert. Jeder Wert besteht aus 16 bits, wobei die letzten 4 bits Kennbits sind. Der RVP6-Wert wird somit nur aus 12 bis errechnet:

            # Ermittle nun den RVP Wert aus den Binärdaten
            for i in range(0, div):

                # schneide den Wert, bestehen aus den ersten 16 bits aus dem bit-string heraus: 
                bytes_value = bits_array[i*16:(i+1)*16]

                # Ist das 14. Bit (no data) oder das 16. Bit (clutter) gesetzt, so setze den Wert auf 0:
                if (bytes_value[10]) or (bytes_value[8]):

                    RVP6_value = 0
                    

                # Ist das 13. Bit gesetzt (Hagel), so setze die Reflektivität auf Maximum.
                # Maximum := (85 dbz + 32,5)*2*10 = RVP62350:
                elif bytes_value[11]:

                    RVP6_value = 2350
                
                else:
                    # Berechne den RVP6 Wert durch:              
                    # Teile das Array in der Mitte, tausche die Arrays aus und setze sie wieder zusammen:
                    byte_value = np.concatenate((bytes_value[8:16],bytes_value[0:8]))

                    # Berechne aus den ersten 12 bits (gelesen von rechts nach links) den RVP6 - Wert (binär to integer)
                    RVP6_value = byte_value[-n:].dot(1 << np.arange(byte_value[-n:].size)[::-1])
                                    
                                     
                # Füge den RVP6 Wert dem RVP6-1D-Array hinzu:
                RVP6_list.append(RVP6_value)

            # Wandle die Liste in ein Array um und forme es entsprechend der Zeilen und Spalten:
            RVP6_array = np.array(RVP6_list).reshape(self.rows, self.columns)          
                
            return RVP6_array[::-1]


        except Exception as e:
            print("============================================================")
            print("Fehler beim Erstellen der Binärteil-Matrix !!!") 
            print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
            print("============================================================")     
            print(e)
            print(sys.exc_info()) 
            print(traceback.format_exc())
            sys.exit(1)




    def create_dbz_array(self, RVP6_array):
        """ 
            1. Nimmt das binary Array entgegen und führt eine Bereinigung durch:
                
            Wandelt die darin enthaltenen Werte in
            Refelktivitätswerte gemäß der Gleichung:

            dbz = Wert (((RVP6)*0.1)/2)-32.5

            um.
        """

        try:
            # siehe Formel RVP6 to dbz
            dbz_array = ((RVP6_array*0.1)/2)-32.5

            # Setze alle Werte welche nagativ sind auf np.nan:
            dbz_array[dbz_array < 1] = np.nan

            return dbz_array

        except Exception as e:
            print("============================================================")
            print("Fehler beim Erstellen der Refletivitäts-Matrix !!!") 
            print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
            print("============================================================")     
            print(e)
            print(sys.exc_info())
            print(traceback.format_exc())
            sys.exit(1)       





    def create_meshgrid(self):

        # Anschließend gilt:
        # phi = latitude
        # lam = longitude

        # Deklaration
        #######################################################
        
        Se_KartCoor = pd.Series(dtype=np.float64)

        # Berechnen der kartesischen Koordinaten der Eckpunkte:
        #######################################################

        try:
            # Geografische Koorinaten der Eckpunkte in [rad]:

            # north-west:
            north_west_lat = self.coords_lat_NW*(np.pi/180)
            north_west_lon = self.coords_lon_NW*(np.pi/180) 

            # north-east:
            north_east_lat = self.coords_lat_NE*(np.pi/180)
            north_east_lon = self.coords_lon_NE*(np.pi/180)

            # unten-links:
            south_east_lat = self.coords_lat_SE*(np.pi/180)
            south_east_lon = self.coords_lon_SE*(np.pi/180)

            # unten-rechts:
            south_west_lat = self.coords_lat_SW*(np.pi/180)
            south_west_lon = self.coords_lon_SW*(np.pi/180)



            Se_geogrCoor = pd.Series({'NW' : [north_west_lat, north_west_lon],
                                      'NE' : [north_east_lat, north_east_lon],
                                      'SE' : [south_east_lat, south_east_lon],
                                      'SW' : [south_west_lat, south_west_lon]})


            

            for point in Se_geogrCoor.index.values:

                # Berechnung des stereografischen Skalierungsfaktors (Mphi):
                Mphi = (1 + np.sin(self.phi_0))/(1 + np.sin(Se_geogrCoor[point][0]))

                x = self.earthRadius * Mphi * np.cos(Se_geogrCoor[point][0]) * np.sin(Se_geogrCoor[point][1] - self.lambda_0)
                y = -self.earthRadius * Mphi * np.cos(Se_geogrCoor[point][0]) * np.cos(Se_geogrCoor[point][1] - self.lambda_0)
        
                Se_KartCoor[point] = [x, y]
             
           
        
            Se_ranges = pd.Series({'lat_max' : np.average(np.array(Se_KartCoor['NW'][1], Se_KartCoor['NE'][1])),
                                   'lat_min' : np.average(np.array(Se_KartCoor['SW'][1], Se_KartCoor['SE'][1])),
                                   'lon_max' : np.average(np.array(Se_KartCoor['NE'][0], Se_KartCoor['SE'][0])),
                                   'lon_min' : np.average(np.array(Se_KartCoor['NW'][0], Se_KartCoor['SW'][0]))}) 


            x_range = np.linspace(Se_ranges['lon_min'], Se_ranges['lon_max'], self.columns)
            y_range = np.linspace(Se_ranges['lat_max'], Se_ranges['lat_min'], self.rows)


            xx, yy = np.meshgrid(x_range, y_range)

            # geografische Koordinaten der Länge des Niederschlagrasters:
            meshgrid_lon = ((np.arctan(-xx/yy)+self.lambda_0)*180)/np.pi

            Z1 = np.power(self.earthRadius,2)
            Z2 = np.power((1+np.sin(self.phi_0)),2)
            Z3 = np.power(xx,2)+np.power(yy,2)

            Z = Z1*Z2-Z3

            N = Z1*Z2+Z3


            # geografische Koordinaten der Breite des Niederschlagrasters:
            meshgrid_lat = (np.arcsin(Z/N)*180)/np.pi

            return {"meshgrid_lon": meshgrid_lon, "meshgrid_lat": meshgrid_lat}


        except Exception as e:
            print("============================================================")
            print("Fehler beim Erstellen des Meshgrids !!!") 
            print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
            print("============================================================")     
            print(e)
            print(sys.exc_info()) 
            print(traceback.format_exc())
            sys.exit(1)



    def create_filename(self):
        """
            Erstellt auf Grundlage der Radarbildeigenschaften einen Dateinamen, unter dem 
            die Datei in der Datenbank referenziert und im entsprechenden Ordner zu finden sein wird. Radar_WN_20210812.json
        """

        try:
            if self.to_produce == "radar_geojson":
                self.filename = "Radar_"+self.product+self.date.strftime('%Y%m%d%H%M')+".geojson"
            elif self.to_produce == "radar_png":
                self.filename = "Radar_"+self.product+self.date.strftime('%Y%m%d%H%M')+".png"


        except Exception as e:
            print("============================================================")
            print("Fehler beim Erstellen des Dateinamens!", self.radararchive) 
            print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
            print("============================================================")     
            print(e)
            print(sys.exc_info()) 
            print(traceback.format_exc())
            sys.exit(1)

    
            
            

    def create_RadarGeoJson(self, meshgrid, dbz_array):

        ''' Erstellt mit Hilfe von 3 Arrays das GeoJson:
            Input: meshgrid :	Dictionary, welche die meshgrids der geogr. Länge und Breite beinhaltet
                  dbz_array :   Array mit den entsprechenden dbz-Werten der Radarreflektivität

             Output: Eine GeoJson Datei
        '''

        try:
            # Entpacken der meshgrid-Arrays:
            arr_latitude = meshgrid["meshgrid_lat"]
            arr_longitude = meshgrid["meshgrid_lon"]

            # Definierden der Array-Idizes:
            lat_max_idx, lon_max_idx = np.shape(dbz_array)	# WN-Radarprodukt: 1100, 1200
            lat_min_idx, lon_min_idx = (0,0)			# 0,0

            # feature-Liste in der die erstellten Polygone abgelegt werden:
            features = []

            # buffer lookup table
            blt = {0: 0.025, 
                  15: 0.020,
                  28: 0.015,
                  42: 0.010,
                  55: 0.005,
                  75: 0.000}

            # simplify lookup table
            slt = {0: 0.005, 
                  15: 0.005,
                  28: 0.005,
                  42: 0.003,
                  55: 0.001,
                  75: 0.000}

            # Radar-Intensitätsklassen:
            # (m,n,k)
            # m...Minimumwert
            # n...Maximumwert
            # k...Klassenbezeichner
            for value in [(0,100,15), (15,100,28), (28,100,42), (42,100,55), (55,100,75), (75,100,85)]:
                
                # Ersetze alle NaN durch -999, da im nächsten Schritt keine Vergleiche mit NaN durchgefüht werden können.
                dbz_array[np.isnan(dbz_array)] = -999

                # Finde alle Werte in den entsprechenden Grenzen
                mask = (dbz_array > value[0]) & (dbz_array <= value[1])

                # Finde die dazugegörigen Koordinaten:
                lat = arr_latitude[mask]
                lon = arr_longitude[mask]

                # Füge die beiden 1 x n Arrays zu einem 2 x n Array zusammen
                coords = np.column_stack((lon,lat))

                # Erstelle aus jedem Koordinatenpaar des 2 x n Arrays einen Punkt mit einem Buffer:
                poly_points = [Point(i[0],i[1]).buffer(0.01) for i in coords]

                # Verbinde alle Polygone der Klasse zu einem Gesamtpolygon:
                unioned_poly_points = unary_union(poly_points)

                # Prüfe, ob die Verbindung der Polygone ein Polygon erzeugt hat und caste es zu einem Multipolygon:
                unioned_poly_points = MultiPolygon([unioned_poly_points]) if (unioned_poly_points.geom_type == 'Polygon') else unioned_poly_points

                #print(value[2],"dbz :",len(unioned_poly_points),"polygons")

                for poly in unioned_poly_points:
                    # Füge Polygon nur hinzu, wenn es Mindestgröße überschreitet (Gilt nur für niedrigste Echos):
                    if (((value[0] == 0) and (poly.area > 0.001)) or (value[0] > 0)):

                        # Erstelle runde Polygongrenzen:
                        poly = poly.buffer(blt[value[0]]).buffer(-2*blt[value[0]]).buffer(blt[value[0]])

                        # Erstellen einer Polygonliste:
                        try:
                            # Nur Polygonflächen:
                            if (len(poly.exterior.coords) > 2):
                                features.append(gj.Feature(geometry=gj.Polygon(mapping(poly.simplify(slt[value[0]], preserve_topology=False))['coordinates'], precision=3), properties={"dbz": value[2]}))
                        except AttributeError:
                            for ply in poly:
                                # Nur Polygonflächen:
                                if (len(ply.exterior.coords) > 2):
                                    features.append(gj.Feature(geometry=gj.Polygon(mapping(ply.simplify(slt[value[0]], preserve_topology=False))['coordinates'], precision=3), properties={"dbz": value[2]}))

            # Erstelle aus den Features eine collection
            feature_collection = gj.FeatureCollection(features)

            # Speichern der fertigen geoJson Datei:
            with open(os.path.join(self.save_dir,self.filename), 'w') as f:
                gj.dump(feature_collection, f)

            # Nochmaliges öffnen und löschen sämtlicher Whitespaces 
            # zur Größenreduzierung der Datei
            with open(os.path.join(self.save_dir,self.filename), 'r+') as js_file:
                minified = jsmin(js_file.read())
                js_file.seek(0)
                js_file.truncate()
                js_file.write(minified)
                js_file.close()


        except Exception as e:
            print("============================================================")
            print("Fehler beim Erstellen der GeoJSON Datei!", self.filename) 
            print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
            print("============================================================")     
            print(e)
            print(sys.exc_info())
            print(traceback.format_exc())
            sys.exit(1)


    


    def create_RadarImage(self, meshgrid, dbz_array):

        ''' Erstellt mit Hilfe von 3 Arrays die Rasterkarte und eine Maske um die Rasterkarte auszuschneiden:
            Input: meshgrid = Dictionary, welche die meshgrids der geogr. Länge und Breite beinhaltet
                   dbz_array = Array mit den entsprechenden dbz-Werten der Radarreflektivität

            Output: Eine Rasterkarte auf einer Basemap mit den entsprechenden 
                    Reflektivitätswerten des Niederschlagsradars.
        ''' 
        
        lon_min = 6.2
        lon_max = 14.6
        lat_min = 47.1
        lat_max = 55
        self.mask_name = "radar_raster_mask_"+self.date.strftime('%Y%m%d%H%M')+".png"


        def create_mask(lon_min, lon_max, lat_min, lat_max):

            ''' Erstellt mit Hilfe einer Shapedatei eine Maske im Format ".png", um das Radarbild auszuschneiden
                Input: Shapedatei von Deutschland (ger_shapefile)
	
            '''
            try:
                # Festlegen der Projektion:
                proj = ccrs.NorthPolarStereo(central_longitude=10.0)

                # Extrahiere die shapes der Shapedatei:
                shapes = list(shpreader.Reader(self.shapefile).geometries())

                # Projektion und Ausmaße der Basemap festlegen:
                ax = plt.axes(projection=proj)
                ax.set_extent([lon_min, lon_max, lat_min, lat_max])

                ax.add_geometries(shapes, ccrs.PlateCarree(), facecolor=(0, 0, 0), edgecolor='none')

                # Entferne den Rahmen um das Bild
                ax.axis("off")

                # Lege die Hintergrundfarbe als transparent fest
                ax.patch.set_visible(False)

                plt.savefig(os.path.join(self.tmp_dir, self.mask_name), format='png', dpi=150, transparent=True, bbox_inches='tight', pad_inches=0)

                plt.clf()
                plt.close()

            except Exception as e:
                print("============================================================")
                print("Fehler beim Erstellen der png Maske: radar_raster_mask.png") 
                print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
                print("============================================================")     
                print(e)
                print(sys.exc_info())
                print(traceback.format_exc())


        def cut_image():

            try:
                # Öffnen des rasterbildes und der dazugehörigen Maske:
                mask = Image.open(os.path.join(self.tmp_dir, self.mask_name))
                preImage = Image.open(os.path.join(self.tmp_dir, self.filename))

                # Umwandlung der Mask in greyscale:
                converted_mask = mask.convert('L')
                converted_mask = converted_mask.point(lambda x: x < 100 and 255)
                img = ImageOps.fit(preImage, converted_mask.size)
                img.putalpha(converted_mask)

                # Abspeichern des ausgeschnittenen Radarbildes
                img.save(os.path.join(self.save_dir, self.filename), optimize=True, quality=95)
                
                # Löschen der Maske und des PreImages:
                os.remove(os.path.join(self.tmp_dir, self.mask_name))
                os.remove(os.path.join(self.tmp_dir, self.filename))

            except Exception as e:
                print("============================================================")
                print("Fehler beim Ausschneiden des Radarrasters:", self.filename) 
                print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
                print("============================================================")
                print(e)
                print(sys.exc_info())
                print(traceback.format_exc())
                sys.exit(1)



        try:
            # Festlegen der Projektion:
            proj = ccrs.NorthPolarStereo(central_longitude=10.0)

            # Extrahiere die shapes der Shapedatei:
            shapes = list(shpreader.Reader(self.shapefile).geometries())

            # Projektion und Ausmaße der Basemap festlegen:
            ax = plt.axes(projection=proj)
            ax.set_extent([lon_min, lon_max, lat_min, lat_max])    

            # Füge die shape-geometrien hinzu: / Hintergrundfarbe: grau / Umrandung der Shapes: none, 
            ax.add_geometries(shapes, ccrs.PlateCarree(), facecolor=(0.2, 0.2, 0.2), edgecolor='none', alpha=1, zorder=0)

            # Kennzeiche bestimmt Städte auf der Karte:
            ax.plot(13.401367, 52.516106, '^', color='#FFFFFF', markersize=1, transform = ccrs.Geodetic())
            ax.text(13.5, 52.4, 'Berlin', color='#FFFFFF', size=4, transform = ccrs.Geodetic())

            ax.plot(8.685294, 50.110564, '^', color='#FFFFFF', markersize=1, transform = ccrs.Geodetic())
            ax.text(8.78, 50.01, 'Frankfurt a. M.', color='#FFFFFF', size=4, transform = ccrs.Geodetic())

            ax.plot(9.988102, 53.538997, '^', color='#FFFFFF', markersize=1, transform = ccrs.Geodetic())
            ax.text(10.08, 53.43, 'Hamburg', color='#FFFFFF', size=4, transform = ccrs.Geodetic())
    
            ax.plot(11.572019, 48.138584, '^', color='#FFFFFF', markersize=1, transform = ccrs.Geodetic())
            ax.text(11.67, 48.03, 'München', color='#FFFFFF', size=4, transform = ccrs.Geodetic())

            ax.plot(6.960396, 50.936782, '^', color='#FFFFFF', markersize=1, transform = ccrs.Geodetic())
            ax.text(7.06, 50.83, 'Köln', color='#FFFFFF', size=4, transform = ccrs.Geodetic())

            # Erstelle eine colormap
            ColorList = ['#009934', '#99cc00', '#ffff00', '#ff8900', '#ff0000', '#0000ff']
            colormap = LinearSegmentedColormap.from_list('DWD', ColorList, 6)

            # Das Raster anhand der Koordinaten-Arrays auf die Basemap plotten:
            im = plt.pcolormesh(meshgrid["meshgrid_lon"], meshgrid["meshgrid_lat"], dbz_array, cmap=colormap, vmax=85, vmin=0, transform=ccrs.PlateCarree())
 
            # Entferne den Rahmen um das Bild
            ax.axis("off")

            #Setze den Hintergrund transparent:
            ax.patch.set_visible(False)

            # Die Karte als png speichern:
            plt.savefig(os.path.join(self.tmp_dir, self.filename), format='png', dpi=150, transparent=True, bbox_inches='tight', pad_inches=0)

            # clear figure
            plt.clf()
            plt.close()

            # Erstelle eine Maske zum Ausscheiden:
            create_mask(lon_min, lon_max, lat_min, lat_max)

            # Schneide das Radarraster mit Hilfe der zuvor erstellten Maske aus und erstellt ein "Radar_latestImage.png" für die Startsete:
            cut_image()
            



        except Exception as e:
            print("============================================================")
            print("Fehler beim Erstellen der png Datei!", self.filename) 
            print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
            print("============================================================")     
            print(e)
            print(sys.exc_info())
            print(traceback.format_exc())
            sys.exit(1)




    def write_dataset_in_db(self, db_host, db_usr, db_pwd, db_name, db_port):

        """
            Schreibt abschließend den Dateinamen der erstellten Datei in die Datenbank.
            Dabei wird der zuvor erstellte Datensatz um den Dateinamen ergänzt.
        """
        
        SQL_write_filename = """UPDATE `Radarbilder` SET Dateiname = :dateiname WHERE Termin = :termin AND Radarprodukt = :product AND Dateiformat = :dateiformat;"""

        try:

            # Verbindung zur Datenbank herstellen:
            connection = check_DB.verbinde_zu_Datenbank(db_host, db_usr, db_pwd, db_name, db_port)

            # erstelle Parameter:
            parameter = {"dateiname": self.filename,
                         "termin": self.str_date,
                         "product": self.product,
                         "dateiformat": self.fileformat}         

            # schreibe Datensatz:
            connection.execute(text(SQL_write_filename), parameter)
            connection.commit()
            connection.close()

        except Exception as e:
            print("============================================================")
            print("Fehler beim Schreiben des Dateinamens in die db!", self.filename) 
            print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
            print("============================================================")     
            print(e)
            print(sys.exc_info())
            print(traceback.format_exc())
            sys.exit(1)






def create(dataset):

    # das "dataset" ist ein tuple, welches 10 Dictionaries beinhaltet dataset[0] - dataset[9]:

    # Entpacken des datasets:
    date = dataset[0]['date']

    str_date = dataset[1]['str_date']

    radararchive = os.path.join(dataset[3]['downloaded_save_dir'], dataset[2]['radararchive'])

    tmp_dir = dataset[4]['tmp_dir']

    save_dir = dataset[5]['save_dir']

    to_produce = dataset[6]['to_produce']

    product = dataset[7]['product']

    shapefile = dataset[8]['shapefile']

    fileformat = dataset[9]['fileformat']

    # Erstellung eines Radarbild-Objekts:
    Radarbild = Radarimage(R=6370.04, 
                           lam=10,
                           phi=60,
                           date=date,
                           str_date=str_date,
                           tmp_dir=tmp_dir,
                           save_dir=save_dir,
                           to_produce=to_produce,
                           product=product,
                           shapefile=shapefile,
                           fileformat=fileformat)

    # Liefert den Inhalt der Radardatei:
    content = Radarbild.unzip_and_read_radarfile(radararchive)
    
    # Ermittelt Informationen zur Radardatei:
    Radarbild.get_fileproperties(content)

    # Bezieht den Binärteil der Datei:
    binary = Radarbild.extract_binary(content)

    # Erstelle ein Array mit den Binärdaten gemäß der Zeilen-/Spaltenanzahl
    RVP6_array = Radarbild.create_RVP6_array(binary)

    # Erstelle aus dem binary array ein Reflektivitätsarray her (in dbz):
    dbz_array = Radarbild.create_dbz_array(RVP6_array)

    # Erstelle ein projeziertes Meshgrid:
    meshgrid = Radarbild.create_meshgrid()
    
    # Erstelle einen Dateinamen:
    Radarbild.create_filename()


    if to_produce == "radar_geojson":
        # Radarbild.create_RadarGeoJson(meshgrid, dbz_array)
        
        # Erstelle eine GeoJSON-Datei:
        # Gib die Arrays und das Radarbild zurück, da die Erstellung
        # ... über multiprocessing in erstelle_Radarfile_JSON geschieht:
        return Radarbild, meshgrid, dbz_array
        
    elif to_produce == "radar_png":
        # Erstelle eine png-Datei:
        Radarbild.create_RadarImage(meshgrid, dbz_array)        
        # Kein Rückgabewert, da die lineare Produktion hier stattfinden kann
        
        # Schreibe abschließend den Dateinamen in die Datenbank:
        Radarbild.write_dataset_in_db(db_host, db_usr, db_pwd, db_name, db_port)          

    # Zeige sämtliche Informationen zum Radarbild an:
    # Radarbild.show_Information()



#########################################################################################################################
# Funktionen für die Herstellung von GEOJSON Dateien.
#########################################################################################################################



def create_GEOJSON_dbzFeatures(meshgrid, dbz_array, dbz_lower_border, dbz_upper_border, dbz_level):
    
    '''
        Erstellt eine Liste mit Featuren für ein bestimmten dbz-Level:
        Input: meshgrid :		Dictionary, welche die meshgrids der geogr. Länge und Breite beinhaltet
               dbz_array :   		Array mit den entsprechenden dbz-Werten der Radarreflektivität
               dbz_lower_border:	Untere Grenze des dbz-Intervalls
               dbz_upper_border:	Obere Grenze der maximal möglichen dbz-Werte (eigtl. immer 100)
               dbz_lebel:		Obere Grenze des dbz-Intervalls (auch Bezeichner des Levels)
        Output: Liste, welche die Features eines bestimmten dbz-Levels (i.d.R. 15, 28, 42, 55, 75, >75) beinhaltet:
    '''
    try:
        # Entpacken der meshgrid-Arrays:
        arr_latitude = meshgrid["meshgrid_lat"]
        arr_longitude = meshgrid["meshgrid_lon"]
        
        # Definierden der Array-Idizes:
        lat_max_idx, lon_max_idx = np.shape(dbz_array)	# WN-Radarprodukt: 1100, 1200
        lat_min_idx, lon_min_idx = (0,0)			# 0,0
            
        # feature-Liste in der die erstellten Polygone abgelegt werden:
        features = []

        # buffer lookup table
        blt = {0: 0.025, 
              15: 0.020,
              28: 0.015,
              42: 0.010,
              55: 0.005,
              75: 0.000}

        # simplify lookup table
        slt = {0: 0.005, 
              15: 0.005,
              28: 0.005,
              42: 0.003,
              55: 0.001,
              75: 0.000} 
                   
        # Ersetze alle NaN durch -999, da im nächsten Schritt keine Vergleiche mit NaN durchgefüht werden können.
        dbz_array[np.isnan(dbz_array)] = -999      
                  
        # Finde alle Werte in den entsprechenden Grenzen
        mask = (dbz_array > dbz_lower_border) & (dbz_array <= dbz_upper_border) 
            
        # Finde die dazugegörigen Koordinaten:
        lat = arr_latitude[mask]
        lon = arr_longitude[mask]

        # Füge die beiden 1 x n Arrays zu einem 2 x n Array zusammen
        coords = np.column_stack((lon,lat))
            
        # Erstelle aus jedem Koordinatenpaar des 2 x n Arrays einen Punkt mit einem Buffer:
        poly_points = [Point(i[0],i[1]).buffer(0.01) for i in coords]

        # Verbinde alle Polygone der Klasse zu einem Gesamtpolygon:
        unioned_poly_points = unary_union(poly_points)

        # Prüfe, ob die Verbindung der Polygone ein Polygon erzeugt hat und caste es zu einem Multipolygon:
        unioned_poly_points = MultiPolygon([unioned_poly_points]) if (unioned_poly_points.geom_type == 'Polygon') else unioned_poly_points     
        
        for poly in unioned_poly_points.geoms:
            # Füge Polygon nur hinzu, wenn es Mindestgröße überschreitet (Gilt nur für niedrigste Echos):

            if (((dbz_lower_border == 0) and (poly.area > 0.001)) or (dbz_lower_border > 0)):

                # Erstelle runde Polygongrenzen:
                poly = poly.buffer(blt[dbz_lower_border]).buffer(-2*blt[dbz_lower_border]).buffer(blt[dbz_lower_border])

                # Erstellen einer Polygonliste:
                try:
                    # Nur Polygonflächen:
                    if (len(poly.exterior.coords) > 2):
                        features.append(gj.Feature(geometry=gj.Polygon(mapping(poly.simplify(slt[dbz_lower_border], preserve_topology=False))['coordinates'], precision=3), properties={"dbz": dbz_level}))
                except AttributeError:
                    for ply in poly.geoms:
                        # Nur Polygonflächen:
                        if (len(ply.exterior.coords) > 2):
                            features.append(gj.Feature(geometry=gj.Polygon(mapping(ply.simplify(slt[dbz_lower_border], preserve_topology=False))['coordinates'], precision=3), properties={"dbz": dbz_level}))
        return features

    except Exception as e:
        print(e)
        print(sys.exc_info())
        print(traceback.format_exc())
        return []



def create_GEOJSON_Files(Radarbild, features2d):

    '''
        Erstellt aus den Featuren der einzelnen dbz-Levels:
        - eine feature Collection
        - 
    '''
    try:   
        # Erstelle aus der 2D Liste eine 1D Liste:
        # [[f150, f151, f152,...],[f280,f281,f282,...],[f420,f421,f422,...]] => [f150,f151,f152, ... ,f280,f281,f282, ... ,f420,f421,f422]
        features1d = [j for i in features2d for j in i]
           
        # Erstelle aus den Features eine collection
        feature_collection = gj.FeatureCollection(features1d)
 
        # Speichern der fertigen geoJson Datei:
        with open(os.path.join(Radarbild.save_dir,Radarbild.filename), 'w') as f:
            gj.dump(feature_collection, f)

        # Nochmaliges öffnen und löschen sämtlicher Whitespaces 
        # zur Größenreduzierung der Datei
        with open(os.path.join(Radarbild.save_dir,Radarbild.filename), 'r+') as js_file:
            minified = jsmin(js_file.read())
            js_file.seek(0)
            js_file.truncate()
            js_file.write(minified)
            js_file.close()
            
        return True    
            
 
    except Exception as e:
        print("============================================================")
        print("Fehler beim Erstellen der GeoJSON Datei!", Radarbild.filename) 
        print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
        print("============================================================")     
        print(e)
        print(sys.exc_info())
        print(traceback.format_exc())
        return False



def write_GEOJSON_dataset_in_db(Radarbild, db_host, db_usr, db_pwd, db_name, db_port):

    """
        Schreibt abschließend den Dateinamen der erstellten Datei in die Datenbank.
        Dabei wird der zuvor erstellte Datensatz um den Dateinamen ergänzt.
    """
        
    SQL_write_filename = """UPDATE `Radarbilder` SET Dateiname = :dateiname WHERE Termin = :termin AND Radarprodukt = :product AND Dateiformat = :dateiformat;"""

    try:

        # Verbindung zur Datenbank herstellen:
        connection = check_DB.verbinde_zu_Datenbank(db_host, db_usr, db_pwd, db_name, db_port)


        # erstelle Parameter:
        parameter = {"dateiname":Radarbild.filename,
                     "termin": Radarbild.str_date,
                     "product": Radarbild.product,
                     "dateiformat": Radarbild.fileformat}           

        # schreibe Datensatz:
        connection.execute(text(SQL_write_filename), parameter)
        connection.commit()
        connection.close()
        
    except Exception as e:
        print("============================================================")
        print("Fehler beim Schreiben des Dateinamens in die db!", Radarbild.filename) 
        print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
        print("============================================================")     
        print(e)
        print(sys.exc_info())
        print(traceback.format_exc())







