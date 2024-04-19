#!/usr/bin/env python3


import os, sys
import numpy as np
import pandas as pd
import traceback
import matplotlib.pyplot as plt
import matplotlib as mpl
import cartopy.crs as ccrs
import cartopy.io.shapereader as shpreader
from matplotlib.colors import LinearSegmentedColormap
from datetime import datetime
from PIL import Image, ImageOps



image_config = {
    "input_dir": f"{os.getcwd()}/output/",				# 
    "output_dir": f"{os.getcwd()}/output/",				# 
    "tmp_dir": f"{os.getcwd()}/tmp/",					# Verzeichnis für Zwischenergebnisse
    "latitude_raster_file": "lat.csv",					# Datei mit den Breitengraden des Rasters
    "longitude_raster_file": "lon.csv",					# Datei mit den Längengraden des Rasters
    "value_raster_file": "interpolRaster_c.csv",			# Datei mit den interpolierten Rasterwerten 
    "germany_shapefile": f"{os.getcwd()}/ger_shapefile/germany.shp",	# Shapefile zum erstellen der Maske
    "germany_mask": f"{os.getcwd()}/ger_mask/germany.png",		# Maske zum Ausschneiden des Bildes
    "image_dpi": 300,							# Auflösung des Bildes
    "image_extents": [5.0, 16.0, 47.0, 55.0],				# Abmaß des Rasters
    "image_proj": ccrs.NorthPolarStereo(central_longitude=10.0),	# Projektion des Rasters
    "image_transform": ccrs.PlateCarree(),
    "lat_raster": "",							# Raster mit den Breitengraden
    "lon_raster": "",							# Raster mit den Längengraden
    "value_raster": ""							# Raster mit den interpolierten Werten
}





class InterpolMap():


    initiated_time = datetime.utcnow()				# Startzeit der Bilddarstellung	


    def __init__(self, image_config):
        
        self.input_dir = image_config.get("input_dir")
        self.output_dir = image_config.get("output_dir")
        self.tmp_dir = image_config.get("tmp_dir")
        self.latitude_raster_file = image_config.get("latitude_raster_file")
        self.longitude_raster_file = image_config.get("longitude_raster_file")
        self.value_raster_file = image_config.get("value_raster_file")
        self.germany_shapefile = image_config.get("germany_shapefile")
        self.germany_mask = image_config.get("germany_mask")
        self.image_dpi = image_config.get("image_dpi")
        self.image_extents = image_config.get("image_extents")
        self.image_proj = image_config.get("image_proj")
        self.lat_raster = image_config.get("lat_raster")
        self.lon_raster = image_config.get("lon_raster")
        self.value_raster = image_config.get("value_raster")
        self.image_transform = image_config.get("image_transform")
        
        
    def show_properties(self):
    
        """
            Zeigt die Eigenschaften der Karteerstellung.
        """
        
        try:
            print("")
            print("------------------------- INFORMATION -------------------------")
            print("initiated: %45s" % (self.initiated_time))
            print("---------------------------------------------------------------")
            print("input_directory: %61s" % (self.input_dir))
            print("output_directory: %60s" % (self.output_dir))
            print("temp_directory: %59s" % (self.tmp_dir))            
            print("latitude raster file: %15s" % (self.latitude_raster_file))
            print("longitude raster file: %14s" % (self.longitude_raster_file))
            print("values raster file: %28s" % (self.value_raster_file))
            print("germany shapefile: %77s" % (self.germany_shapefile))           
            print("=================================================================================================")        
            print("")
            return {"success": True}
            
        except Exception as e:
            return {"success": False, "err": e, "msg": traceback.format_exc()}    






    def read_rasters(self):

        """
            Ließt die 3 Raster mit Längenkoordinaten (X) und Breitenkoordinaten (Y) und den interpolierten Werten.
        """

        try:
        
            # Lese die lat und lon csv-Dateien im Inputordner und erstelle 2 Raster X (Länge) und Y (Breite)
            # Existenz des Rasters mit den geogr. Breitengraden prüfen:
            if (self.latitude_raster_file in os.listdir(self.input_dir)):
                
                # Existenz des Rasters mit den geogr. Längengraden prüfen:
                if (self.longitude_raster_file in os.listdir(self.input_dir)):
                
                    # Einlesen der drei Raster:
                    # --------------------------------------
                    
                    # Einlesen Breitengrade
                    self.lat_raster = pd.read_csv(os.path.join(self.input_dir,self.latitude_raster_file), sep=";", header=None, dtype=np.float64 , decimal=".").values
                    
                    # Einlesen Längengrade
                    self.lon_raster = pd.read_csv(os.path.join(self.input_dir,self.longitude_raster_file), sep=";", header=None, dtype=np.float64 , decimal=".").values

                    # Einlesen interpolierte Werte
                    self.value_raster = pd.read_csv(os.path.join(self.input_dir,self.value_raster_file), sep=";", header=None, dtype=np.float64 , decimal=".").values                    
                  
                    return {"success": True}

                else:
                    Exception("Fehler!",f"{self.latitude_raster_file} nicht vorhanden.")
            
            else:
                raise Exception("Fehler!",f"{self.longitude_raster_file} nicht vorhanden.")
    
        except Exception as e:
            return {"success": False, "err": e, "msg": traceback.format_exc()}





    def create_mask(self):

        ''' Erstellt mit Hilfe einer Shapedatei eine Maske im Format ".png", um das Rasterbild auszuschneiden
        Input: Shapedatei von Deutschland (ger_shapefile)
	
        '''
        
        try:
            # Einlesen des Shapefiles:
            shapes = list(shpreader.Reader(self.germany_shapefile).geometries())


            # Projektion und Ausmaße der Basemap festlegen:
            ax = plt.axes(projection=self.image_proj)
            ax.set_extent(self.image_extents)

            ax.add_geometries(shapes, self.image_transform, facecolor=(0, 0, 0), edgecolor='none')


            # Entferne den Rahmen um das Bild
            ax.axis("off")
            ax.patch.set_visible(False)

            # Speichern der Maske im entsprechenden Ordner:
            plt.savefig(self.germany_mask, format='png', dpi=self.image_dpi, transparent=True, bbox_inches='tight', pad_inches=0)

            plt.clf()
            plt.close()
            
            return {"success": True}
        
        except Exception as e:
            return {"success": False, "err": e, "msg": traceback.format_exc()}




    def cutImage(self):
    
        # Lade die Maske
        mask = Image.open(self.germany_mask)
        
        # Lade das erstellte Bild
        preImage = Image.open(os.path.join(self.tmp_dir,"preImage.png"))

        try:
            # Umwandlung der Mask in greyscale:
            converted_mask = mask.convert('L')

            converted_mask = converted_mask.point(lambda x: x < 100 and 255)

            img = ImageOps.fit(preImage, converted_mask.size)

            img.putalpha(converted_mask)

            img.save(os.path.join(self.output_dir,"InterpolImage.png"), optimize=True, quality=95)

            return {"success": True}
                   
        except Exception as e:
            return {"success": False, "err": e, "msg": traceback.format_exc()}
            
            

    def create_image(self):
    
        """
            Erstellt aus 3 Rastern (Breitengrade, Längengrade, Werte) ein Rasterimage (.png).
        """
        
        try:
            
            # Wurde die Maske erfolgreich erstellt:
            mask_created = self.create_mask()
            
            if (mask_created.get("success")):
            
                # Lade die shapedatei
                shapes = list(shpreader.Reader(self.germany_shapefile).geometries())

                # Lade die Projektion
                ax = plt.axes(projection=self.image_proj)
                ax.set_extent(self.image_extents)
            
                # Das Raster anhand der Koordinaten-Arrays auf die Basemap plotten:
                im = plt.pcolormesh(self.lon_raster, 
                                    self.lat_raster, 
                                    self.value_raster, 
                                    cmap='YlGnBu_r', 
                                    vmax=np.amax(self.value_raster), 
                                    vmin=np.amin(self.value_raster), 
                                    transform=self.image_transform)        
            
                # Entferne den Rahmen um die Karte:
                ax.axis("off")
            
                ax.patch.set_visible(False)            
            
                # Die Karte als png speichern:
                plt.savefig(os.path.join(self.tmp_dir,"preImage.png"), format='png', dpi=self.image_dpi, transparent=True, bbox_inches='tight', pad_inches=0)

                # clear figure
                plt.clf()
                plt.close()            
            
                # Schneide das Bild mit Hilfe der Maske aus.
                image_cut = self.cutImage()
                if (image_cut.get("success")):
                
                    return {"success": True}
                
                else:
                    raise Exception(image_cut.get("err"), image_cut.get("msg"))
            else:
                raise Exception(mask_created.get("err"), mask_created.get("msg"))
            
        
        except Exception as e:
            return {"success": False, "err": e, "msg": traceback.format_exc()}










def create_image(image_config):

    

    try:
        Map = InterpolMap(image_config)
        
        # Zeige eine Zusammenfassung des aktuellen Bildes:
        properties_shown = Map.show_properties()
        
        
        
        if (properties_shown.get("success")):
        
            rasters_read = Map.read_rasters()
        
        else:
            raise Exception(properties_shown.get('err'), properties_shown.get('msg'))
            
        #==============================================================================            
            
        if (rasters_read.get("success")):
        
            image_created = Map.create_image()
        
        else:
            raise Exception(rasters_read.get("err"), rasters_read.get("msg"))
            
        #==============================================================================                        
            
        if not (image_created.get("success")):
        
            raise Exception(image_created.get("err"), image_created.get("msg"))        

    except Exception as e:
        print("============================================================")
        print("Fehler beim Erstellen des Rasterimages")
        for err in e.args: print(err)
        print("============================================================") 






create_image(image_config)

















