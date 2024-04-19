#!/opt/WetterDE/env/bin/python3


import ftplib, sys, os, re
import pandas as pd
import time
import numpy as np
from scipy.interpolate import Rbf
import matplotlib.pyplot as plt


import cartopy.io.shapereader as shpreader
import cartopy.crs as ccrs

from PIL import Image, ImageOps
from matplotlib.colors import LinearSegmentedColormap




def createTemporallyInterpolatedArrays(interpolated_arrays):

    keys = list(interpolated_arrays.keys())
    new_interpolated_arrays = {}


    for i in range(len(keys)):

        try:
            if i+1 < len(keys):

                newKey = keys[i] + pd.Timedelta(minutes=30)

                newArray = (interpolated_arrays[keys[i]][2] + interpolated_arrays[keys[i+1]][2]) * 0.5
                
                new_interpolated_arrays[keys[i]] = [interpolated_arrays[keys[i]][0], interpolated_arrays[keys[i]][1], interpolated_arrays[keys[i]][2]]

                new_interpolated_arrays[newKey] = [interpolated_arrays[keys[i]][0], interpolated_arrays[keys[i]][1], newArray]

            else:

                new_interpolated_arrays[keys[i]] = [interpolated_arrays[keys[i]][0], interpolated_arrays[keys[i]][1], interpolated_arrays[keys[i]][2]]

        except Exception as e:
            print("Fehler beim zeitlichen Interpolieren der Arrays")
            print(e)
            print(sys.exc_info())
    
    print(len(new_interpolated_arrays.keys()),"Termine werden erstellt...\n")
           
    return new_interpolated_arrays




def createInterpolatedArray(data):


    # Definition der Interpolationsparameter:
    # methode = 'cubic'					#: r**3
    # 'multiquadric': sqrt((r/self.epsilon)**2 + 1)
    # 'inverse': 1.0/sqrt((r/self.epsilon)**2 + 1)
    # 'gaussian': exp(-(r/self.epsilon)**2)
    methode = 'linear' 					#: r
    # 'quintic': r**5
    # methode = 'thin_plate'				#: r**2 * log(r)

    KartenAuflösung = 0.01 # Minimum: 0.01

    # Eigenschaften der Fläche:
    lon_min = 5.86
    lon_max = 15.05
    lon_res  = KartenAuflösung

    lat_min = 47.27
    lat_max = 55.06
    lat_res = np.negative(KartenAuflösung)

    smooth = 0.0
    epsilon = 5

    interpolated_arrays = {}    

    ################################################
    ################################################


    def createMeshgrid(lon_min, lon_max, lon_res, lat_min, lat_max, lat_res):


        # Erstellen der x- und y-Achsen des Meshgrids:
        lat_axes = np.arange(lat_max, lat_min, lat_res)		# Breite
        lon_axes = np.arange(lon_min, lon_max, lon_res)  	# Länge

        # Mesh:
        X,Y = np.meshgrid(lon_axes, lat_axes)
    
        return X, Y





    ################################################


    def interpolateArray(X, Y, lon, lat, val, methode, smooth, epsilon):

            
        rbf = Rbf(lon, lat, val, function=methode, smooth=smooth, epsilon=epsilon)

        arr = rbf(X,Y)

        return arr

    ################################################


    for key in data.groups.keys():

        dataset = data.get_group(key)


        # Koordinaten
        lat = dataset['lat'].values		# geogr. Breite
        lon = dataset['lon'].values		# geogr. Länge
        
        # Wert:
        val = dataset['value'].values

        try:
            # get mesh:
            X, Y = createMeshgrid(lon_min, lon_max, lon_res, lat_min, lat_max, lat_res)

        except Exception as e:
            print(sys.exc_info())
            print(e)
            continue

        try:
            # interpolate:
            int_array = interpolateArray(X, Y, lon, lat, val, methode, smooth, epsilon)

            # Eintrag für Dict
            interpolated_arrays[key] = [X, Y, int_array]

        except Exception as e:
            print(sys.exc_info())
            print(e)
            continue

    return interpolated_arrays




def createRasterImage(interpolated_arrays, parameter, shapefile_germany, tmp_dir, save_dir):

    ''' Erstellt mit Hilfe von 3 Arrays die Karte:
    Input: log = Array mit den geogr. Längstwerten
           lat = Array mit den geogr. Breitenwerten
           values = Array mit den entsprechenden Werten

    Output: Eine Rasterkarte auf einer Basemap

    '''


    # Definitionen:  
    proj = ccrs.NorthPolarStereo(central_longitude=10.0)
    shapes = list(shpreader.Reader(shapefile_germany).geometries())
    extents = [6.2, 14.6, 47.1, 55]
    #ColorList = ['#a50026','#d73027','#f46d43','#fdae61','#fee090','#ffffbf','#d9ef8b','#a6d96a','#66bd63','#1a9850',\
    #             '#eff3ff','#c6dbef','#9ecae1','#6baed6','#4292c6','#2171b5','#8c6bb1','#88419d','#810f7c','#4d004b']

    #ColorList = ['#ff009a','#ff009a','#ff009a','#ff009a','#ff009a','#ff006c','#ff003e','#ff0000','#ff2100','#ff3100',
    #             '#ff3d00','#ff4700','#ff5f00','#ff7400','#ff8600','#ff9700','#ffb300','#f9bf22','#f4ca38','#efd44c',
    #             '#ebde5f','#e7e772','#e5ef85','#e3f798','#e3ffab','#c6e993','#aad37b','#8dbd65','#70a84f','#53943a','#337f25','#006b0f',
    #             '#f7fbff','#d6e5f4','#b7cee9','#98b8df','#7aa1d4','#5c8bc9','#3b75be','#0060b2',
    #             '#2254ac','#2d4ea9','#3747a5','#3f40a0','#47399b','#4e3095','#54278f','#7a0177','#8b2e88','#9c4b99',
    #             '#ac66aa','#bd80bb','#cd9acc','#deb4dd','#eeceee','#ffe9ff','#ffe9ff','#ffe9ff','#ffe9ff','#ffe9ff','#ffe9ff','#ffe9ff','#ffe9ff','#ffe9ff']

    ColorList = ['#f200ff','#d80060','#7d0101','#a70805','#d21404','#ff2100','#ff4f00','#ff6d00','#ff8700','#ff9e00',
                 '#ffb300','#f6c530','#eed650','#e8e56e','#e4f38d','#b1d881','#8bbc63','#65a047','#3d852c','#006b0f',
                 '#f7fbff','#bad9ff','#83b6ff','#5090fc','#2468f3','#325ff9','#4454fd','#5846ff','#8100ff','#8100ff',
                 '#a000fa','#b900f5','#cf00f0','#e212eb','#f324e7','#f65eee','#fba2f8','#fdbffc','#fdbffc','#fedbfe']

    ColorList.reverse()

    colormap = LinearSegmentedColormap.from_list('DWD', ColorList, 320)

    

    def create_mask(shapefile_germany, tmp_dir):

        ''' Erstellt mit Hilfe einer Shapedatei eine Maske im Format ".png", um das Rasterbild auszuschneiden
        Input: Shapedatei von Deutschland (ger_shapefile)
	
        '''

        # Festlegen der Projektion:  
        proj = ccrs.NorthPolarStereo(central_longitude=10.0)
        shapes = list(shpreader.Reader(shapefile_germany).geometries())

        # Projektion und Ausmaße der Basemap festlegen:
        ax = plt.axes(projection=proj)
        ax.set_extent([6.2, 14.6, 47.1, 55])

        ax.add_geometries(shapes, ccrs.PlateCarree(), facecolor=(0, 0, 0), edgecolor='none')

        # Entferne den Rahmen um das Bild
        ax.axis("off")

        # Lege die Hintergrundfarbe als transparent fest
        # ax.background_patch.set_visible(False)
        ax.patch.set_visible(False)

        plt.savefig(os.path.join(tmp_dir, "maske.png"), format='png', dpi=150, transparent=True, bbox_inches='tight', pad_inches=0)

        plt.clf()
        plt.close()



    def cutImage(index, interp_arrays_length, key, parameter, tmp_dir, save_dir):
        print(key)
        mask = Image.open(os.path.join(tmp_dir,"maske.png"))
        preImage = Image.open(os.path.join(tmp_dir,"preImage.png"))
        filename = parameter+key.strftime('_%Y-%m-%d-%H%M.png')
        datetime = key.strftime('%Y-%m-%d %H:%M:00')

        try:
            # Umwandlung der Mask in greyscale:
            converted_mask = mask.convert('L')

            converted_mask = converted_mask.point(lambda x: x < 100 and 255)

            img = ImageOps.fit(preImage, converted_mask.size)

            img.putalpha(converted_mask)

            img.save(os.path.join(save_dir,filename), optimize=True, quality=95)

            # Vom aktuellsten Bild soll ein "latestImage" erstellt werden:
            if index == interp_arrays_length-1:
                img.save(os.path.join(save_dir,"Temperatur2m_latestImage.png"), optimize=True, quality=95)

            return {"Termin":datetime, "Parameter":parameter, "Dateiformat":"png", "Dateiname":filename}
                   
        except Exception as e:
            print(e)
            print(sys.exc_info())

    ###########################################################
    ###########################################################


    # Erstellung einer Maske zum Ausschneiden des Rasters:
    create_mask(shapefile_germany, tmp_dir)

    ###########################################################

    text_size = 4
    images_to_write_in_db = pd.DataFrame(columns=['Termin','Parameter','Dateiformat','Dateiname'])
    intArrlen = len(interpolated_arrays)	# Länge des Dictionarys mit den 
 
    for index, key in enumerate(interpolated_arrays.keys()):

        try: 
            # nimm Datensatz:
            dataset = interpolated_arrays[key]

            lon = dataset[0]
            lat = dataset[1]
            values = dataset[2]
        

            ###########################################################
 
            # Projektion und Ausmaße der Basemap festlegen:
            ax = plt.axes(projection=proj)
            ax.set_extent(extents)

            # Füge die shape-geometrien hinzu: / Hintergrundfarbe: grau / Umrandung der Shapes: none, 
            ax.add_geometries(shapes, ccrs.PlateCarree(), facecolor=(0.2, 0.2, 0.2), edgecolor='none', alpha=1, zorder=0)

            # Kennzeichne bestimmt Städte auf der Karte:
            ax.plot(13.401367, 52.516106, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(13.5, 52.4, 'Berlin', color='#000000', size=text_size, transform = ccrs.Geodetic())

            ax.plot(8.685294, 50.110564, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(8.78, 50.01, 'Frankfurt a. M.', color='#000000', size=text_size, transform = ccrs.Geodetic())

            ax.plot(9.988102, 53.538997, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(10.08, 53.43, 'Hamburg', color='#000000', size=text_size, transform = ccrs.Geodetic())
    
            ax.plot(11.572019, 48.138584, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(11.67, 48.03, 'München', color='#000000', size=text_size, transform = ccrs.Geodetic())

            ax.plot(6.960396, 50.936782, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(7.06, 50.83, 'Köln', color='#000000', size=text_size, transform = ccrs.Geodetic())

            ax.plot(11.633568, 52.124336, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(11.73, 52.22, 'Magdeburg', color='#000000', size=text_size, transform = ccrs.Geodetic())

            ax.plot(12.370985, 51.346787, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(12.47, 51.24, 'Leipzig', color='#000000', size=text_size, transform = ccrs.Geodetic())

            ax.plot(13.739751, 51.048577, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(13.83, 50.94, 'Dresden', color='#000000', size=text_size, transform = ccrs.Geodetic())

            ax.plot(11.032111, 50.983170, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(11.13, 50.88, 'Erfurt', color='#000000', size=text_size, transform = ccrs.Geodetic())

            ax.plot(11.078412, 49.444632, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(11.17, 49.34, 'Nürnberg', color='#000000', size=text_size, transform = ccrs.Geodetic())

            ax.plot(9.178556, 48.773485, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(9.27, 48.67, 'Stuttgart', color='#000000', size=text_size, transform = ccrs.Geodetic())

            ax.plot(6.985210, 49.235611, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(7.18, 49.33, 'Saarbrücken', color='#000000', size=text_size, transform = ccrs.Geodetic())

            ax.plot(9.733955, 52.387419, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(9.83, 52.28, 'Hannover', color='#000000', size=text_size, transform = ccrs.Geodetic())

            ax.plot(8.830905, 53.080614, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(8.73, 53.18, 'Bremen', color='#000000', size=text_size, transform = ccrs.Geodetic())

            ax.plot(8.045263, 52.274018, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(8.14, 52.17, 'Osnabrück', color='#000000', size=text_size, transform = ccrs.Geodetic())

            ax.plot(12.098614, 54.088992, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(12.19, 53.98, 'Rostock', color='#000000', size=text_size, transform = ccrs.Geodetic())

            ax.plot(9.519159, 51.312480, '^', color='#000000', markersize=1, transform = ccrs.Geodetic())
            ax.text(9.61, 51.21, 'Kassel', color='#000000', size=text_size, transform = ccrs.Geodetic())

            # Das Raster anhand der Koordinaten-Arrays auf die Basemap plotten:
            im = plt.pcolormesh(lon, lat, values, cmap=colormap, vmax=40, vmin=-40, transform=ccrs.PlateCarree())

            # Erstelle die 5'er Intervall-Level für die Contourlines:
            lvls1 = np.arange(-40, 40, 5)

            # Zeichne die 5'er Intervall-Level:
            contour1 = plt.contour(lon, lat, values, levels=lvls1, colors='#242424', linewidths=0.1, transform=ccrs.PlateCarree())

            # Zeichne die Labels der 5'er Intervall-Level:
            clabels1 = plt.clabel(contour1, inline=True, fontsize=5, fmt="%d", colors='#242424', rightside_up=True, use_clabeltext=True) 
           
            # Rotiere alle labels so, dass sie aufrecht stehen::
            for label in contour1.labelTexts:
                label.set_rotation(0)


            # Erstelle die 1'er Intervall-Level für die Contourlines, entferne aber die 5'er Intervalle:
            lvls2 = [item for item in np.arange(-40, 40, 1) if item % 5 != 0]

            # Zeichne die 1'er Intervall-Level:
            contour2 = plt.contour(lon, lat, values, levels=lvls2, linestyles="dotted", colors='#242424', linewidths=0.1, transform=ccrs.PlateCarree())
            







            # Entferne den Rahmen um die Karte:
            ax.axis("off")
            
            #Setze den Hintergrund transparent:
            #ax.background_patch.set_visible(False)
            ax.patch.set_visible(False)

            # Die Karte als png speichern:
            plt.savefig(os.path.join(tmp_dir,"preImage.png"), format='png', dpi=150, transparent=True, bbox_inches='tight', pad_inches=0)

            # clear figure
            plt.clf()
            plt.close()


            # Sollte bis hierhin alles gut verlaufen sein, so füge die Informationen der erstellten Rasterbilder dem Dataframe hinzu:
            #images_to_write_in_db = images_to_write_in_db.append(cutImage(index, intArrlen, key, parameter, tmp_dir, save_dir), ignore_index=True)
            images_to_write_in_db = pd.concat([images_to_write_in_db, pd.DataFrame.from_records([cutImage(index, intArrlen, key, parameter, tmp_dir, save_dir)])], ignore_index=True, verify_integrity=True)

        except Exception as e:
            print(e)
            print(sys.exc_info())
            continue

    return images_to_write_in_db











