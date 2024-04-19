#!/opt/WetterDE/env/bin/python3



import sys, os
import pandas as pd
import numpy as np
import traceback
import ftplib
import re
import Qmet

Configverzeichnis = '/opt/WetterDE/config'
sys.path.append(Configverzeichnis)
import konfiguration

Modulverzeichnis = konfiguration.Module.module_dir
sys.path.append(Modulverzeichnis)

import check_DB
import time
from datetime import datetime, timedelta
from sqlalchemy import text

########################################################################
########################################################################
ftp_server_name = konfiguration.obsData_GER.ftp_server_name		# Name des FTP-Servers des DWD, von dem die Daten gedownloaded werden
ftp_server_usr = konfiguration.obsData_GER.ftp_server_usr		# Benutzername mit dem ich mich auf dem FTP-Server des DWD anmelde:
ftp_server_pwd = konfiguration.obsData_GER.ftp_server_pwd		# Password mit dem ich mich auf dem FTP-Server des DWD anmelde:

csv_quelle = konfiguration.obsData_GER.ger_synop_quelle		# Das Verzeichnis auf dem FTP-Server auf dem die csv-Dateien zu finden sind
csv_ziel = konfiguration.obsData_GER.ger_synop_save_dir		# Das Verzeichnis in das die gedownloadeten csv-Dateien auf MeteoOnline abgelegt werden:		

db_name = konfiguration.Datenbank.name
db_host = konfiguration.Datenbank.host
db_usr = konfiguration.Datenbank.usr
db_pwd = konfiguration.Datenbank.pwd
db_port = konfiguration.Datenbank.port

########################################################################
########################################################################




class ObsDataset():

    """
        Ist eine Kollektion aller relevanten Datensätze.
    """
    
    dataset_initiated = datetime.utcnow()		# Wann wurde die Erstellung des Datensatzes eingeleitet.
    country = "GER"					# Länderkennung des Datensatzes
    lst_ger_kennziffern = ""				# Liste von deutschen Stationskennziffern, welche in der Tabelle "Sattionskennziffern", aber nicht in der "Ignorelist" sind
    lst_ignorelist_kennziffern = ""			# Liste von Stations_ids die auf der Ignorelist stehen.
    
    df_summary_csv_to_download = pd.DataFrame(columns=['Kennziffer','Tag','Monat','Zeit','Datei', 'Bytes'])
    df_summary_csv_successfully_downloaded = pd.DataFrame(columns=['Kennziffer','Tag','Monat','Zeit','Datei', 'Bytes'])

    def __init__(self, ftp_server_name, ftp_server_usr, ftp_server_pwd, csv_quelle, csv_ziel, db_host, db_usr, db_pwd, db_name, db_port):
        
        self.ftp_server_name = ftp_server_name
        self.ftp_server_usr = ftp_server_usr
        self.ftp_server_pwd = ftp_server_pwd
        self.csv_quelle = csv_quelle
        self.csv_ziel = csv_ziel
        self.db_host = db_host
        self.db_usr = db_usr
        self.db_pwd = db_pwd
        self.db_name = db_name
        self.db_port = db_port



    def show_properties(self):
    
        """
            Zeigt die Eigenschaften des Datenabrufs.
        """
        
        try:
            print("")
            print("------------------REQUEST INFORMATION-------------------------")
            print("dataset_initiated: %29s" % (self.dataset_initiated))
            print("--------------------------------------------------------------")
            print("ftp_server_name: %20s" % (self.ftp_server_name))
            print("ftp_server_usr: %15s" % (self.ftp_server_usr))
            print("ftp_server_pwd: %25s" % (self.ftp_server_pwd))
            print("--------------------------------------------------------------")
            print("csv_quelle: %39s" % (self.csv_quelle))
            print("csv_ziel: %52s" % (self.csv_ziel))
            print("--------------------------------------------------------------")
            print("country: %16s" % (self.country))
            print("=================================================================================================")        
            print("")
            return {"success": True}
            
        except Exception as e:
            return {"success": False, "err": e, "msg": traceback.format_exc()}




    def send_startmessage(self):
    
        """
            Sendet die Startmeldung des Datenabrufs.
        """
        try:
            print("=================================================")
            print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
            print(f'Starte Abruf der Beobachtungsdaten (self.country)')
            print("Abruf von opendata.dwd.de...")
            return {"success": True}
            
        except Exception as e:
            return {"success": False, "err": e, "msg": traceback.format_exc()}




    def get_stations_from_DB(self):
    
        """
            Liefert alle Stations_ids von Deutschland, welche in der Tabelle Stationskennziffern, aber...
            nicht in der Tabelle "Ignorelist" sind.
        """
        
        sql = """SELECT Stationskennziffern.Kennziffer as kennziffer
                   FROM Stationskennziffern 
                  WHERE Stationskennziffern.Laenderkennung = :country AND
                        Stationskennziffern.Kennziffer NOT IN (SELECT Ignorelist.Kennziffer FROM Ignorelist);"""
        try:                
        
            print("Rufe Stationkennziffern ab ...")
        
            # Stelle eine Verbindung zur Datenbank her:
            with check_DB.verbinde_zu_Datenbank(self.db_host, self.db_usr, self.db_pwd, self.db_name, self.db_port) as connection:

                parameter = {"country":self.country}
            
                self.lst_ger_kennziffern = list(pd.read_sql(sql=text(sql), con=connection, params=parameter)["kennziffer"].values)
            
                print("erfolgreich.\n")
            
                return {"success": True}
            
        except Exception as e:
            return {"success": False, "err": e, "msg": traceback.format_exc()}
             
             
             
             
    def get_stations_from_ignorelist(self):
     
        """
            Liefert alle Stations_ids welche in der Tabelle "Ignorelist" sind.
        """
        
        sql = """SELECT Ignorelist.Kennziffer as kennziffer
                   FROM Ignorelist;"""
               
               
        try:
        
            print("Rufe Stationskennziffern von Ignorelist ab ...")
        
            # Stelle eine Verbindung zur Datenbank her:
            with check_DB.verbinde_zu_Datenbank(self.db_host, self.db_usr, self.db_pwd, self.db_name, self.db_port) as connection:      
        
                self.lst_ignorelist_kennziffern = list(pd.read_sql(sql=text(sql), con=connection)["kennziffer"].values)
            
                print("erfolgreich.\n")
                
                return {"success": True}
            
        except Exception as e:
            return {"success": False, "err": e, "msg": traceback.format_exc()}        
             
             
             
                
    def download_csvFiles(self):

        """
            Aufgabe:	Erstellt einen Dataframe des Inhalts des ftp-Verzeichnisses:
            		--> "Kennziffer" | "Tag" | "Monat" | "Zeit" | "Datei" | "Bytes"
            		
            Rückgabe: 	o.g. Dataframe       
        """
        
        # Liste für den unformatierten Inhalt des ftp-Verzeichnisses:
        lst_ftp_content = []
        
        try:
        
            print("Erstelle Downloadliste ...")
        
            # Anonymes anmelden auf dem FTP Server:
            with ftplib.FTP(self.ftp_server_name, self.ftp_server_usr, self.ftp_server_pwd) as ftp_connection:
            
                # Wechseln in das ftp-Verzeichnis:
                ftp_connection.cwd(self.csv_quelle) 
        
                # Inhalt des Verzeichnisses der Liste "lst_ftp_content" übergeben:
                ftp_connection.retrlines('LIST', lst_ftp_content.append)


                for entry in lst_ftp_content:
            
                    # splitten des Eintrags:
                    dataset = entry.split()
                
                    # Ermittle die Stationskennziffer des Datensatz:
                    kennziffer = re.split('-|_',dataset[8])[0]
                
                    # ist die Kennziffer in der Tabelle "Stationskennziffern" und nicht in der Tabelle "Ignorelist",
                    # So füge Sie dem Dataframe ftp_content hinzu:
                    if ((kennziffer in self.lst_ger_kennziffern) and (kennziffer not in self.lst_ignorelist_kennziffern)):
                    
                        data = {"Kennziffer":kennziffer, 
                                "Tag": dataset[6],
                                "Monat": dataset[5],
                                "Zeit": dataset[7],
                                "Datei": dataset[8],
                                "Bytes": dataset[4]}
                               
                     
                        self.df_summary_csv_to_download = pd.concat([self.df_summary_csv_to_download, pd.DataFrame.from_records([data])], ignore_index=True, verify_integrity=True)
                        
                    else:
                        continue

                print("erfolgreich.\n")
            
        
        
                if not self.df_summary_csv_to_download.empty:   
            
                    print("Downloade csv-Dateien ...")
            
                    for row in self.df_summary_csv_to_download.itertuples(index=True, name='Pandas'):
            
                        try: 
                            # Eine Datei im Downloadordner binär öffnen:   
                            with open(os.path.join(self.csv_ziel,row.Datei), 'wb') as Datei:
                
                                # Daten binär downloaden und in die binär geöffnete Datei kopieren:
                                ftp_connection.retrbinary('RETR '+row.Datei, Datei.write)
                
                
                
                                dataset = {"Kennziffer":row.Kennziffer, 
                                           "Tag": row.Tag,
                                           "Monat": row.Monat,
                                           "Zeit": row.Zeit,
                                           "Datei": row.Datei,
                                           "Bytes": row.Bytes}
                
                        
                                self.df_summary_csv_successfully_downloaded = pd.concat([self.df_summary_csv_successfully_downloaded, pd.DataFrame.from_records([dataset])], ignore_index=True, verify_integrity=True)
                
                        except Exception as e:
                            print("-------------------------------------------")
                            print(f'Fehler beim downloaden der csv-Datei: {row.Datei}')
                            print(e)
                            print("-------------------------------------------")
                            continue
            
                    print("erfolgreich.\n")
                    return{"success": True}
                          
                else:
                    raise Exception("Es gibt keine aktuellen csv-Dateien zu downloaden.","Vorgang wird beendet.")
                    
        except Exception as e:            
            return {"success": False, "err": e, "msg": traceback.format_exc()}
            
            
            
    def schreibe_Stundenwerte_in_Datenbank(self): 
     
        """
            Holt die Datensätze der letzten 30h aus der Datenbank.
            Vergleicht die Daten aus der Datenbank mit denen der csv-Dateien.
            -> Schreibt neue Datensätze in die Datenbank.
            -> Schreibt aktualisierte Datensätze in die Datenbank.
            -> Schreibt die Tageswerte, welche um 06 UTC gemeldet werden in die Datenbank.
        """
         
        sql_hole_Datensaetze_DB = """SELECT Stundenwerte.Termin as "Termin",
                                            Stundenwerte.Temperatur2m as "Temperatur (2m)",
                                            Stundenwerte.Taupunkt2m as "Taupunkttemperatur (2m)",
                                            Stundenwerte.Bodendruck as "Druck (auf Meereshoehe)",
                                            Stundenwerte.Niederschlagssumme as "Niederschlag (letzte Stunde)",
                                            Stundenwerte.RelativeFeuchte as "Relative Feuchte",
                                            Stundenwerte.Sichtweite as "Sichtweite",
                                            Stundenwerte.Sonnenscheindauer as "Sonnenscheindauer (letzte Stunde)",
                                            Stundenwerte.Wolkenbedeckung as "Wolkenbedeckung",
                                            Stundenwerte.Windgeschwindigkeitmean10min as "Windgeschwindigkeit",
                                            Stundenwerte.Windgeschwindigkeitmax1h as "Windboen (letzte Stunde)",
                                            Stundenwerte.Windrichtung as "Windrichtung",
                                            Stundenwerte.WetterAktuell as "aktuelles Wetter",
                                            Stundenwerte.Schneehoehe as "Schneehoehe"
                                       FROM Stationskennziffern inner join Stundenwerte on (Stationskennziffern.Kennziffer = Stundenwerte.Kennziffer)
                                      WHERE Stundenwerte.Termin >= DATE_ADD(NOW(), INTERVAL -30 HOUR) AND
                                            Stundenwerte.Kennziffer = :kennziffer
                                   ORDER BY Stundenwerte.Termin DESC;"""
         
         
        rel_header = ['Datum','Uhrzeit (UTC)','Wolkenbedeckung','Temperatur (2m)','Taupunkttemperatur (2m)','Sichtweite', 'Windboen (letzte Stunde)', 'Windrichtung', 'Windgeschwindigkeit', 'Niederschlag (letzte Stunde)','aktuelles Wetter','Druck (auf Meereshoehe)','Relative Feuchte','Schneehoehe','Sonnenscheindauer (letzte Stunde)']

        ord_header = ['Termin','Temperatur (2m)','Taupunkttemperatur (2m)','Druck (auf Meereshoehe)', 'Niederschlag (letzte Stunde)','Relative Feuchte','Sichtweite','Sonnenscheindauer (letzte Stunde)', 'Wolkenbedeckung', 'Windgeschwindigkeit','Windboen (letzte Stunde)', 'Windrichtung', 'aktuelles Wetter','Schneehoehe']

        input_header = ['Termin','Datum','Messzeit','Kennziffer','Temperatur (2m)','Taupunkttemperatur (2m)','Druck (auf Meereshoehe)', 'Niederschlag (letzte Stunde)','Relative Feuchte','Sichtweite','Sonnenscheindauer (letzte Stunde)', 'Wolkenbedeckung', 'Windgeschwindigkeit','Windboen (letzte Stunde)', 'Windrichtung', 'aktuelles Wetter','Schneehoehe'] 
     
        only_data_header = ['Temperatur (2m)', 'Taupunkttemperatur (2m)', 'Druck (auf Meereshoehe)', 'Niederschlag (letzte Stunde)', 'Relative Feuchte', 'Sichtweite', 'Sonnenscheindauer (letzte Stunde)', 'Wolkenbedeckung', 'Windgeschwindigkeit', 'Windboen (letzte Stunde)', 'Windrichtung', 'aktuelles Wetter', 'Schneehoehe']     
     
     
        try:
            # Wurden csv-Dateien heruntergeladen:
            if not self.df_summary_csv_successfully_downloaded.empty:         
        
                print("-------------------------------------------------")
                print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
                print(f'Stundenwerte ({self.country}) werden verarbeitet...')
                Startzeit = time.time()
            
                # Verbindung zur Datenbank:
                with check_DB.verbinde_zu_Datenbank(self.db_host, self.db_usr, self.db_pwd, self.db_name, self.db_port) as connection: 
     
                    # Zähler für die neu geschriebenen Stundenwerte:
                    Stundenwerte_Neu = 0
            
                    # Zähler für die aktualisierten Stundenwerte:
                    Stundenwerte_Akt = 0
                
                    # List mit Stationskennziffern ohne Daten / nur NaN im DataFrame:
                    lst_stations_missing_data = []
                    
                    # List mit Stationskennziffern mit Daten:
                    lst_stations_data = []  
                             
                    # Öffne jede heruntergeladene Datei der Liste der Reihe nach:
                    for row in self.df_summary_csv_successfully_downloaded.itertuples(index=True, name='Pandas'):

                        try:
                            # Lade die Datensätze dieser Station der letzten 30h:
                            parameter = {"kennziffer": row.Kennziffer}
                            df_Datensaetze_DB = pd.read_sql(sql=text(sql_hole_Datensaetze_DB), params=parameter, con=connection, index_col='Termin')
                
                            # Öffne die dazugehörige csv-Datei:
                            df_Dateiinhalt = pd.read_csv(os.path.join(self.csv_ziel, row.Datei), sep=';', header=2)
     
                            # Reduzieren des DataFrames auf die relevanten Spalten und ersetzen der '---' in NaN und Kommas in Punkte:
                            df_Datei_relcol = df_Dateiinhalt[rel_header].replace(to_replace='---', value=np.nan).replace(to_replace=',',value='.', regex=True)
                
                            # Erstellen der Spalte "Termin"
                            df_Datei_relcol['Termin']=pd.to_datetime(df_Datei_relcol['Datum']+"-"+df_Datei_relcol['Uhrzeit (UTC)'], format='%d.%m.%y-%H:%M')
                    
                            # Reduzieren des DataFrames auf die relevanten Spalten:
                            # Setze die Spalte 'Termin' als Index
                            # Lösche die Spalte 'Termin'
                            # Caste den DataFrame als float
                            df_Datei = df_Datei_relcol[ord_header].set_index(df_Datei_relcol['Termin']).drop('Termin', axis=1).astype(float).dropna(how='all')
                        
                            # Ist der DataFrame leer, so nimm die nächste Station:
                            if df_Datei.empty:
                            
                                print("Station:",row.Kennziffer,"liefert keine Daten.")
                                lst_stations_missing_data.append(row.Kennziffer)
                                continue
                                
                            else:                
                                ################################################################################################################
                                # Ermitteln der neuen Datensätze:
                                ################################################################################################################                
                
                                # Bilden der Vereinigungsmenge des aktuellen und der alten Datensätze:
                                df_con_Datei_DB = pd.concat([df_Datei, df_Datensaetze_DB],axis=0)

                                # Löschen der doppelten Datensätze:
                                df_dd_con_Datei_DB = df_con_Datei_DB[~df_con_Datei_DB.index.duplicated(keep=False)]

                                # Bilden der Vereinigungsmenge aus den alten Datensätze und den zuvor Ermittelten:
                                df_con_Datei_DB_DB = pd.concat([df_dd_con_Datei_DB, df_Datensaetze_DB],axis=0)

                                # Erneutes Bilden der Vereinigungsmenge aus den alten Datensätze und den zuvor Ermittelten:
                                df_con_Datei_DB_DB_DB = pd.concat([df_con_Datei_DB_DB, df_Datensaetze_DB],axis=0) 
            
                                # Löschen der doppelten Datensätze:
                                df_dd_con_Datei_DB_DB_DB = df_con_Datei_DB_DB_DB[~df_con_Datei_DB_DB_DB.index.duplicated(keep=False)]

                                # Ersetze alle 'nan' Werte durch None: Dadurch werden alle Werte von float zu Objects gecastest:
                                df_neue_Datensaetze = df_dd_con_Datei_DB_DB_DB.where((pd.notnull(df_dd_con_Datei_DB_DB_DB)), other=None).astype(object)

                                # Mache aus den Indizes eine Spalte "Termin"
                                df_input_Dataset = df_neue_Datensaetze.reset_index(level=['Termin'])

                                # Erstelle aus der Spalte "Termin" die Spalte "Datum"
                                df_input_Dataset['Datum'] = pd.to_datetime(df_input_Dataset['Termin'].dt.date)

                                # Erstelle aus der Spalte "Termin" die Spalte "Messzeit"
                                df_input_Dataset['Messzeit'] = df_input_Dataset['Termin'].dt.time

                                # Erstelle die Spalte "Kennziffer"
                                df_input_Dataset['Kennziffer'] = str(row.Kennziffer)

                                # Im Datensatz alle NaN durch None ersetzen
                                # Datensatz auf objects casten
                                df_input_Dataset = df_input_Dataset.astype(object).where((pd.notnull(df_input_Dataset)), None)              
                            
                                ################################################################################################################
                                # Ermitteln der aktualisierten Datensätze:
                                ################################################################################################################ 
                
                                # Als erstes löschen wir die neuen Datensätze aus der Datei, da diese keine Veränderungen aufweisen können:
                                df_verDS_Datei = df_Datei.drop(df_neue_Datensaetze.index.values)

                                # Vereinigungsmenge des zuvor Ermittelten Datensatzes mit den alten Datensaetzen:
                                df_con_verDS_Datei_DB = pd.concat([df_verDS_Datei,df_Datensaetze_DB], axis=0)

                                # Löschen der doppelten Datensätze:
                                df_dd_con_verDS_Datei_DB = df_con_verDS_Datei_DB[~df_con_verDS_Datei_DB.index.duplicated(keep=False)]

                                # Nochmalige Vereinigungsmenge des zuvor Ermittelten Datensatzes mit den alten Datensaetzen:
                                df_con_verDS_DB = pd.concat([df_dd_con_verDS_Datei_DB,df_Datensaetze_DB], axis=0)

                                # Löschen der doppelten Datensätze:
                                df_verDS_DB = df_con_verDS_DB[~df_con_verDS_DB.index.duplicated(keep=False)]

                                # Casten der beiden DataFrames zu floats und ersetzen der None durch NaN:            
                                df_verDS_Datei_None = df_verDS_Datei.where((pd.notnull(df_verDS_Datei)), other=np.nan).astype(float).sort_index(ascending=True)
                                df_verDS_DB_None = df_verDS_DB.where((pd.notnull(df_verDS_DB)), other=np.nan).astype(float).sort_index(ascending=True)             
                
                
                                for i in range(0,len(df_verDS_DB_None.index.values)):

                                    # Prüfe die Werte aus der Datei auf klimatologische Grenzwerte und ...
                                    # ... übersetze das gemeldet ww in den wmo-ww code:
                                    input_Series = pd.Series(data={
                                        "Temperatur (2m)": Qmet.MetObsValue(parameter="tt", value=df_verDS_Datei_None.iloc[i].get("Temperatur (2m)", default=np.nan)).check(country="GER", alt_value=np.nan),
                                        "Taupunkttemperatur (2m)": Qmet.MetObsValue(parameter="tp", value=df_verDS_Datei_None.iloc[i].get("Taupunkttemperatur (2m)", default=np.nan)).check(country="GER", alt_value=np.nan),
                                        "Druck (auf Meereshoehe)": Qmet.MetObsValue(parameter="pp", value=df_verDS_Datei_None.iloc[i].get("Druck (auf Meereshoehe)", default=np.nan)).check(country="GER", alt_value=np.nan),
                                        "Niederschlag (letzte Stunde)": Qmet.MetObsValue(parameter="rr", value=df_verDS_Datei_None.iloc[i].get("Niederschlag (letzte Stunde)", default=np.nan)).check(country="GER", alt_value=np.nan),
                                        "Relative Feuchte": Qmet.MetObsValue(parameter="rh", value=df_verDS_Datei_None.iloc[i].get("Relative Feuchte", default=np.nan)).check(country="GER", alt_value=np.nan),
                                        "Sichtweite": Qmet.MetObsValue(parameter="vv", value=df_verDS_Datei_None.iloc[i].get("Sichtweite", default=np.nan)).check(country="GER", alt_value=np.nan, input_unit="km"),
                                        "Sonnenscheindauer (letzte Stunde)": Qmet.MetObsValue(parameter="sd", value=df_verDS_Datei_None.iloc[i].get("Sonnenscheindauer (letzte Stunde)", default=np.nan)).check(country="GER", alt_value=np.nan, input_unit="minute"),
                                        "Wolkenbedeckung": Qmet.MetObsValue(parameter="nn", value=df_verDS_Datei_None.iloc[i].get("Wolkenbedeckung", default=np.nan)).check(country="GER", alt_value=np.nan, input_unit="percent"),
                                        "Windgeschwindigkeit": Qmet.MetObsValue(parameter="wg", value=df_verDS_Datei_None.iloc[i].get("Windgeschwindigkeit", default=np.nan)).check(country="GER", alt_value=np.nan, input_unit="kmh"),
                                        "Windboen (letzte Stunde)": Qmet.MetObsValue(parameter="wb", value=df_verDS_Datei_None.iloc[i].get("Windboen (letzte Stunde)", default=np.nan)).check(country="GER", alt_value=np.nan, input_unit="kmh"),
                                        "Windrichtung": Qmet.MetObsValue(parameter="wr", value=df_verDS_Datei_None.iloc[i].get("Windrichtung", default=np.nan)).check(country="GER", alt_value=np.nan, input_unit="grad"),
                                        "aktuelles Wetter": Qmet.MetObsValue(parameter="ww", value=df_verDS_Datei_None.iloc[i].get("aktuelles Wetter", default=np.nan)).check(country="GER", alt_value=np.nan),
                                        "Schneehoehe": Qmet.MetObsValue(parameter="snh", value=df_verDS_Datei_None.iloc[i].get("Schneehoehe", default=np.nan)).check(country="GER", alt_value=np.nan)
                                    })

                    
                                    if not np.allclose(input_Series, df_verDS_DB_None.iloc[i], equal_nan=True):
                    
                                        # Im Datensatz alle NaN durch None ersetzen
                                        # Datensatz auf objects casten
                                        Se_None = input_Series.astype(object).where(pd.notnull(input_Series), None)

                                        # Spalte "Kennziffer" einfügen
                                        Se_None["Kennziffer"] = str(row.Kennziffer)

                                        # Spalte "Datum" einfügen
                                        Se_None["Datum"] = df_verDS_Datei_None.iloc[i].name.strftime('%Y-%m-%d')

                                        # Spalte "Termin" einfügen
                                        Se_None["Termin"] = df_verDS_Datei_None.iloc[i].name.strftime('%Y-%m-%d %H:%M:%S')

                                        # Spalte "Messzeit" einfügen
                                        Se_None["Messzeit"] = df_verDS_Datei_None.iloc[i].name.strftime('%H:%M')

                                        sql_Datensatz_aktualisieren_Stundenwerte = "UPDATE Stundenwerte SET Termin = :termin, Datum = :datum, Messzeit = :messzeit, Kennziffer = :kennziffer, Temperatur2m = :tt, Taupunkt2m = :tp, Bodendruck = :pp, Niederschlagssumme = :rr, RelativeFeuchte = :rh, Sichtweite = :vv, Sonnenscheindauer = :sd, Wolkenbedeckung = :nn, Windgeschwindigkeitmean10min = :wg, Windgeschwindigkeitmax1h = :wb, Windrichtung= :wr, WetterAktuell = :ww, Schneehoehe = :snh WHERE Kennziffer = :kennziffer AND Termin = :termin;"
                                        
                                        try:
                                            parameter = {"termin":Se_None.get('Termin'),
                                                         "datum":Se_None.get('Datum'),
                                                         "messzeit":Se_None.get('Messzeit'),
                                                         "kennziffer":Se_None.get('Kennziffer'),
                                                         "tt":Se_None.get('Temperatur (2m)'),
                                                         "tp":Se_None.get('Taupunkttemperatur (2m)'),
                                                         "pp":Se_None.get('Druck (auf Meereshoehe)'),
                                                         "rr":Se_None.get('Niederschlag (letzte Stunde)'),
                                                         "rh":Se_None.get('Relative Feuchte'),
                                                         "vv":Se_None.get('Sichtweite'),
                                                         "sd":Se_None.get('Sonnenscheindauer (letzte Stunde)'),
                                                         "nn":Se_None.get('Wolkenbedeckung'),
                                                         "wg":Se_None.get('Windgeschwindigkeit'),
                                                         "wb":Se_None.get('Windboen (letzte Stunde)'),
                                                         "wr":Se_None.get('Windrichtung'),
                                                         "ww":Se_None.get('aktuelles Wetter'),
                                                         "snh":Se_None.get('Schneehoehe')}

                                        
                                            connection.execute(text(sql_Datensatz_aktualisieren_Stundenwerte), parameter)
                                            Stundenwerte_Akt += 1
                                            
                                        except Exception as e:
                                            print("Fehler beim aktualisieren der Stundenwerte:")
                                            print("-------------------------------------------")
                                            print(parameter)
                                            print(e)
                                            print("-------------------------------------------")
                                            continue

                                # Übernehmen der aktualierten Daten.
                                connection.commit()
                                
                                
                                
                                # Schreiben der neuen Datensätze:
                                sql_Datensatz_einfuegen_Stundenwerte = "INSERT INTO `Stundenwerte` (`Termin`,`Datum`,`Messzeit`,`Kennziffer`,`Temperatur2m`,`Taupunkt2m`,`Bodendruck`,`Niederschlagssumme`,`RelativeFeuchte`,`Sichtweite`,`Sonnenscheindauer`, `Wolkenbedeckung`,`Windgeschwindigkeitmean10min`,`Windgeschwindigkeitmax1h`,`Windrichtung`,`WetterAktuell`,`Schneehoehe`) VALUES (:termin,:datum,:messzeit,:kennziffer,:tt,:tp,:pp,:rr,:rh,:vv,:sd,:nn,:wg,:wb,:wr,:ww,:snh);"


                                for i in df_input_Dataset.index.values:
                                
                                    # Besteht der Datensatz nur aus None?
                                    if not df_input_Dataset.iloc[i][only_data_header].isnull().all():
                                    
                                        try:
                                            # Definieren der Parameter:
                                            parameter = {"termin":df_input_Dataset.iloc[i]['Termin'].strftime('%Y-%m-%d %H:%M:%S'),
                                                         "datum":df_input_Dataset.iloc[i]['Datum'].strftime('%Y-%m-%d'),
                                                         "messzeit":df_input_Dataset.iloc[i]['Messzeit'].strftime('%H:%M'),
                                                         "kennziffer":str(df_input_Dataset.iloc[i]['Kennziffer']),
                                                         "tt":Qmet.MetObsValue(parameter="tt", value=df_input_Dataset.iloc[i].get('Temperatur (2m)')).check(country="GER", alt_value=None),
                                                         "tp":Qmet.MetObsValue(parameter="tp", value=df_input_Dataset.iloc[i].get('Taupunkttemperatur (2m)')).check(country="GER", alt_value=None),
                                                         "pp":Qmet.MetObsValue(parameter="pp", value=df_input_Dataset.iloc[i].get('Druck (auf Meereshoehe)')).check(country="GER", alt_value=None),
                                                         "rr":Qmet.MetObsValue(parameter="rr", value=df_input_Dataset.iloc[i].get('Niederschlag (letzte Stunde)')).check(country="GER", alt_value=None),
                                                         "rh":Qmet.MetObsValue(parameter="rh", value=df_input_Dataset.iloc[i].get('Relative Feuchte')).check(country="GER", alt_value=None),
                                                         "vv":Qmet.MetObsValue(parameter="vv", value=df_input_Dataset.iloc[i].get('Sichtweite')).check(country="GER", alt_value=None, input_unit="km"),
                                                         "sd":Qmet.MetObsValue(parameter="sd", value=df_input_Dataset.iloc[i].get('Sonnenscheindauer (letzte Stunde)')).check(country="GER", alt_value=None, input_unit="minute"),
                                                         "nn":Qmet.MetObsValue(parameter="nn", value=df_input_Dataset.iloc[i].get('Wolkenbedeckung')).check(country="GER", alt_value=None, input_unit="percent"),
                                                         "wg":Qmet.MetObsValue(parameter="wg", value=df_input_Dataset.iloc[i].get('Windgeschwindigkeit')).check(country="GER", alt_value=None, input_unit="kmh"),
                                                         "wb":Qmet.MetObsValue(parameter="wb", value=df_input_Dataset.iloc[i].get('Windboen (letzte Stunde)')).check(country="GER", alt_value=None, input_unit="kmh"),
                                                         "wr":Qmet.MetObsValue(parameter="wr", value=df_input_Dataset.iloc[i].get('Windrichtung')).check(country="GER", alt_value=None, input_unit="grad"),
                                                         "ww":Qmet.MetObsValue(parameter="ww", value=df_input_Dataset.iloc[i].get('aktuelles Wetter')).check(country="GER", alt_value=None),
                                                         "snh":Qmet.MetObsValue(parameter="snh", value=df_input_Dataset.iloc[i].get('Schneehoehe')).check(country="GER", alt_value=None)}


                                            connection.execute(text(sql_Datensatz_einfuegen_Stundenwerte), parameter)
                                            Stundenwerte_Neu += 1
                                            
                                        except Exception as e:
                                            print("Fehler beim einfügen neuer Stundenwerte:")
                                            print("-------------------------------------------")
                                            print(parameter)
                                            print(e)
                                            print("-------------------------------------------")
                                            continue
                        
                                # Übernehmen aller eingefügten neuen Datensätze:
                                connection.commit()
                                
                        except Exception as e:
                            print(f'Fehler bei der Verarbeitung der Stundendaten.')
                            print("-------------------------------------------")
                            print(f'Station: {row.Kennziffer}')
                            print(e)
                            print("-------------------------------------------")
                            
                    connection.commit()            
                    connection.close()
                    
                print("erfolgreich.\n")
                        
                Endzeit = time.time()
                
                print("Zusammenfassung:")
                print("----------------")
                print("Stundenwerte in",'{:.2f}'.format(Endzeit-Startzeit),"Sekunden verarbeitet")
                print(f'Neue Stundenwerte geschrieben: {Stundenwerte_Neu}')
                print(f'Alte Stundenwerte aktualisiert: {Stundenwerte_Akt}')
                print(f'Anzahl an Stationen ohne Daten: {len(lst_stations_missing_data)}')
                print(f'Anzahl an Stationen in Ignoreliste: {len(self.lst_ignorelist_kennziffern)}')
                print("=================================================")
                return {"success": True}         
     
            else:
                print("-------------------------------------------------")
                print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
                print("Es wurden keine csv-Dateien für den Import der Stundendaten heruntergeladen.")
                print("Vorgang wird beendet.")
                print("=================================================")
                return {"success": True} 
                
        except Exception as e:
            return {"success": False, "err": e, "msg": traceback.format_exc()}
 

    def schreibe_Tageswerte_in_Datenbank(self): 
     
        """
            Holt die Datensätze der letzten 3 Tage aus der Datenbank.
            Vergleicht die Daten aus der Datenbank mit denen der csv-Dateien.
            -> Schreibt neue Datensätze in die Datenbank.
            -> Schreibt aktualisierte Datensätze in die Datenbank.
            -> Schreibt die Tageswerte, welche um 06 UTC gemeldet werden in die Datenbank.
        """
         
        sql_hole_Tagesdaten = """SELECT Tageswerte.Datum as "Datum",
                                        Tageswerte.Mitteltemperatur as "mittlere Temperatur (vergangener Tag, 2m)",
                                        Tageswerte.Maximumtemperatur as "Maximumtemperatur (vergangener Tag, 2m)",
                                        Tageswerte.Minimumtemperatur as "Minimumtemperatur (vergangener Tag, 2m)",
                                        Tageswerte.Niederschlagssumme as "Niederschlag (letzte 24 Stunden)",
                                        Tageswerte.Sonnenscheindauer as "Sonnenscheindauer (vergangener Tag)",
                                        Tageswerte.MaxWindgeschwindigkeit as "Maximalwind (vergangener Tag)",
                                        Tageswerte.MaxWindboe as "Windboen (vergangener Tag)"
			           FROM Stationskennziffern inner join Tageswerte on (Stationskennziffern.Kennziffer = Tageswerte.Kennziffer)
                                  WHERE Tageswerte.Datum >= DATE_ADD(NOW(), INTERVAL -3 DAY) AND
                                        Tageswerte.Kennziffer = :kennziffer;"""

 
        rel_header = ['Datum','Uhrzeit (UTC)','mittlere Temperatur (vergangener Tag, 2m)','Maximumtemperatur (vergangener Tag, 2m)', 'Minimumtemperatur (vergangener Tag, 2m)','Niederschlag (letzte 24 Stunden)','Sonnenscheindauer (vergangener Tag)', 'Maximalwind (vergangener Tag)','Windboen (vergangener Tag)']    
     
     
        try:
            # Wurden csv-Dateien heruntergeladen:
            if not self.df_summary_csv_successfully_downloaded.empty:         
        
                print("-------------------------------------------------")
                print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
                print(f'Tageswerte ({self.country}) werden verarbeitet...')
                Startzeit = time.time()
            
                # Verbindung zur Datenbank:
                with check_DB.verbinde_zu_Datenbank(self.db_host, self.db_usr, self.db_pwd, self.db_name, self.db_port) as connection:   
     
                    # Zähler für die neu geschriebenen Stundenwerte:
                    Tageswerte_Neu = 0
            
                    # Zähler für die aktualisierten Stundenwerte:
                    Tageswerte_Akt = 0
                    
                    # List mit Stationskennziffern ohne Daten / nur NaN im DataFrame:
                    lst_stations_missing_data = []
                    
                    # List mit Stationskennziffern mit Daten:
                    lst_stations_data = []                      
            
                    # Öffne jede heruntergeladene Datei der Reihe nach:
                    for row in self.df_summary_csv_successfully_downloaded.itertuples(index=True, name='Pandas'):              
                
                        try:
                            # Lade die Datensätze dieser Station der letzten 3 Tage:
                            parameter = {"kennziffer": row.Kennziffer}
                            df_Tagesdatensaetze_DB = pd.read_sql(sql=text(sql_hole_Tagesdaten), params=parameter, con=connection, index_col='Datum')

                            # Öffne die csv.-Datei:
                            df_Dateiinhalt = pd.read_csv(os.path.join(self.csv_ziel, row.Datei), sep=';', header=2)

                            # Reduzieren des DataFrames auf die relevanten Spalten und ersetzen der '---' in NaN und Kommas in Punkte:
                            df_Dateiinhalt_relcol = df_Dateiinhalt[rel_header].replace(to_replace='---', value=np.nan).replace(to_replace=',',value='.', regex=True)
                
                            # Ziehe jetzt von dem Datum in der Spalte "Datum" einen Tag ab:
                            df_Dateiinhalt_relcol['Datum'] = (pd.to_datetime(df_Dateiinhalt_relcol['Datum'],dayfirst=True, format='%d.%m.%y')-timedelta(days=1)).dt.date

                            # Nimm nun die Zeilen, welche als Uhrzeit '06:00' UTC besitzen:
                            # Lösche die Spalte Uhrzeit:
                            df_Datei_select = df_Dateiinhalt_relcol.loc[df_Dateiinhalt_relcol['Uhrzeit (UTC)'] == '06:00'].drop('Uhrzeit (UTC)', axis=1)

                            # Setzen des neuen Datums als Index:
                            # Löschen der Spalte "Datum" und "Datum_neu"
                            df_Datei = df_Datei_select.set_index(df_Datei_select['Datum']).drop('Datum', axis=1).astype(float).dropna(how='all')           
                
                            # Ist der DataFrame leer, so nimm die nächste Station:
                            if df_Datei.empty:
                            
                                print("Station:",row.Kennziffer,"liefert keine Daten.")
                                lst_stations_missing_data.append(row.Kennziffer)
                                continue
                                
                            else:                
                
                                ################################################################################################################
                                # Ermitteln der neuen Datensätze:
                                ################################################################################################################
            
                                # 1. Vereinigungsmenge von Datensätzen aus Datenbank und Datei:
                                df_con_DB_Datei = pd.concat([df_Datei, df_Tagesdatensaetze_DB],axis=0)

                                # Löschen der doppelten Datensätze:
                                df_dd_con_DB_Datei = df_con_DB_Datei[~df_con_DB_Datei.index.duplicated(keep=False)]

                                # Doppeltes Vereinigen mit den Datensätzen aus der Datenbank:
                                df_con_DB_Datei_DB = pd.concat([df_dd_con_DB_Datei, df_Tagesdatensaetze_DB],axis=0)
                                df_con_DB_Datei_DB_DB = pd.concat([df_con_DB_Datei_DB, df_Tagesdatensaetze_DB],axis=0)

                                # Löschen der doppelten Datensätze:
                                df_dd_con_DB_Datei_DB_DB = df_con_DB_Datei_DB_DB[~df_con_DB_Datei_DB_DB.index.duplicated(keep=False)]

                                # Ersetze alle 'nan' Werte durch None: Dadurch werden alle Werte von float zu Objects gecastest:
                                df_dd_con_DB_Datei_DB_DB_None = df_dd_con_DB_Datei_DB_DB.where((pd.notnull(df_dd_con_DB_Datei_DB_DB)), other=None)

                                # Mache aus den Inizes eine Spalte "Termin"
                                df_neu_Tagesdatensatz = df_dd_con_DB_Datei_DB_DB_None.reset_index(level=['Datum'])

                                # Im Datensatz alle NaN durch None ersetzen
                                # Datensatz auf objects casten
                                df_neu_Tagesdatensatz = df_neu_Tagesdatensatz.astype(object).where((pd.notnull(df_neu_Tagesdatensatz)),None)
 

                                ################################################################################################################
                                # Ermitteln der aktualisierten Datensätze:
                                ################################################################################################################


                                # Als erstes löschen wir die neuen Datensätze aus der Datei, da diese keine Veränderungen aufweisen können:
                                df_verDS_Datei = df_Datei.drop(df_dd_con_DB_Datei_DB_DB.index.values)

                                # Vereinigungsmenge des zuvor Ermittelten Datensatzes mit den alten Datensaetzen:
                                df_con_Datei_DB = pd.concat([df_verDS_Datei,df_Tagesdatensaetze_DB], axis=0)

                                # Löschen der doppelten Datensätze:
                                df_dd_con_Datei_DB = df_con_Datei_DB[~df_con_Datei_DB.index.duplicated(keep=False)]
                        
                                # Nochmalige Vereinigungsmenge des zuvor Ermittelten Datensatzes mit den alten Datensaetzen:
                                df_con_Datei_DB_DB = pd.concat([df_dd_con_Datei_DB,df_Tagesdatensaetze_DB], axis=0)

                                # Löschen der doppelten Datensätze: 
                                df_verDS_DB = df_con_Datei_DB_DB[~df_con_Datei_DB_DB.index.duplicated(keep=False)]
            
                                # Casten der beiden DataFrames zu floats und ersetzen der None durch NaN:
                                df_verDS_Datei_None = df_verDS_Datei.where((pd.notnull(df_verDS_Datei)), other=np.nan).astype(float)
                                df_verDS_DB_None = df_verDS_DB.where((pd.notnull(df_verDS_DB)), other=np.nan).astype(float)                
                
                                for i in range(0,len(df_verDS_DB_None.index.values)):
                             
                                    if not np.allclose(df_verDS_Datei_None.iloc[i], df_verDS_DB_None.iloc[i],equal_nan=True):

                                        try:
                                            # Im Datensatz alle NaN durch None ersetzen
                                            # Datensatz auf objects casten
                                            Se_None = df_verDS_Datei_None.iloc[i].astype(object).where((pd.notnull(df_verDS_Datei_None.iloc[i])),None)
                    
                                            # Spalte "Datum" einfügen
                                            Se_None["Datum"] = Se_None.name.strftime('%Y-%m-%d')
                                            
                                            # Datensatz aktualisieren:
                                            sql_Datensatz_aktualisieren_Tageswerte = "UPDATE Tageswerte SET Datum = :datum, Kennziffer = :kennziffer, Mitteltemperatur = :ttavg, Maximumtemperatur = :ttmax, Minimumtemperatur = :ttmin, Niederschlagssumme = :rr, Sonnenscheindauer = :sd, MaxWindgeschwindigkeit = :wgmax, MaxWindboe = :wb WHERE Kennziffer = :kennziffer AND Datum = :datum;"
                                            parameter = {"datum":Se_None['Datum'],
                                                         "kennziffer":str(row.Kennziffer),
                                                         "ttavg":Se_None.get('mittlere Temperatur (vergangener Tag, 2m)'),
                                                         "ttmax":Se_None.get('Maximumtemperatur (vergangener Tag, 2m)'),
                                                         "ttmin":Se_None.get('Minimumtemperatur (vergangener Tag, 2m)'),
                                                         "rr":Se_None.get('Niederschlag (letzte 24 Stunden)'),
                                                         "sd":Se_None.get('Sonnenscheindauer (vergangener Tag)'),
                                                         "wgmax":Se_None.get('Maximalwind (vergangener Tag)'),
                                                         "wb":Se_None.get('Windboen (vergangener Tag)')}
                    

                                            connection.execute(text(sql_Datensatz_aktualisieren_Tageswerte), parameter)
                                            Tageswerte_Akt += 1
                                            
                                        except Exception as e:
                                            print("Fehler beim aktualisieren der Tageswerte:")
                                            print("-------------------------------------------")
                                            print(parameter)
                                            print(e)
                                            print("-------------------------------------------")
                                            continue
                                            
                                connection.commit() 
                                                   
                                # Den neuen Tagesdatensatz in die Datenbank schreiben:
                                sql_Datensatz_einfuegen_Tageswerte = "INSERT INTO `Tageswerte` (`Datum`,`Kennziffer`,`Mitteltemperatur`,`Maximumtemperatur`,`Minimumtemperatur`,`Niederschlagssumme`,`Sonnenscheindauer`, `MaxWindgeschwindigkeit`,`MaxWindboe`) VALUES (:datum,:kennziffer,:ttavg,:ttmax,:ttmin,:rr,:sd,:wgmax,:wb);"

                                for i in range(0,len(df_neu_Tagesdatensatz.index.values)):

                                    try:
                                        # Parameter definieren
                                        parameter = {"datum":df_neu_Tagesdatensatz.iloc[i].get('Datum').strftime('%Y-%m-%d'),
                                                     "kennziffer":str(row.Kennziffer),
                                                     "ttavg":df_neu_Tagesdatensatz.iloc[i].get('mittlere Temperatur (vergangener Tag, 2m)'),
                                                     "ttmax":df_neu_Tagesdatensatz.iloc[i].get('Maximumtemperatur (vergangener Tag, 2m)'),
                                                     "ttmin":df_neu_Tagesdatensatz.iloc[i].get('Minimumtemperatur (vergangener Tag, 2m)'),
                                                     "rr":df_neu_Tagesdatensatz.iloc[i].get('Niederschlag (letzte 24 Stunden)'),
                                                     "sd":df_neu_Tagesdatensatz.iloc[i].get('Sonnenscheindauer (vergangener Tag)'),
                                                     "wgmax":df_neu_Tagesdatensatz.iloc[i].get('Maximalwind (vergangener Tag)'),
                                                     "wb":df_neu_Tagesdatensatz.iloc[i].get('Windboen (vergangener Tag)')}


                                        connection.execute(text(sql_Datensatz_einfuegen_Tageswerte), parameter)
                                        Tageswerte_Neu += 1
                                        
                                    except Exception as e:
                                        print("Fehler beim einfügen der Tageswerte:")
                                        print("-------------------------------------------")
                                        print(parameter)
                                        print(e)
                                        print("-------------------------------------------")
                                        continue
                                        
                                # Alle neuen und aktualisierten Tageswerte wurden geschrieben.
                                connection.commit()
                                
                        except Exception as e:
                            print(f'Fehler bei der Verarbeitung der Tagesdaten.')
                            print("-------------------------------------------")
                            print(f'Station: {row.Kennziffer}')
                            print(e)
                            print("-------------------------------------------") 
                                                           
                    connection.commit()            
                    connection.close()
                    
                print("erfolgreich.\n")
                        
                Endzeit = time.time()
                print("Zusammenfassung:")
                print("----------------")
                print("Tageswerte in",'{:.2f}'.format(Endzeit-Startzeit),"Sekunden verarbeitet")
                print(f'Neue Tageswerte geschrieben: {Tageswerte_Neu}')
                print(f'Alte Tageswerte aktualisiert: {Tageswerte_Akt}')
                print(f'Anzahl an Stationen ohne Daten: {len(lst_stations_missing_data)}')
                print(f'Anzahl an Stationen in Ignoreliste: {len(self.lst_ignorelist_kennziffern)}')
                print("=================================================")
                return {"success": True}


            else:
                raise Exception("Es wurden keine csv-Dateien für den Import der Tagesdaten heruntergeladen.","Vorgang wird beendet.")
                
                
        except Exception as e:
            return {"success": False, "err": e, "msg": traceback.format_exc()}



                
    def clean(self):
    
        """
            Löscht alle csv-Dateien im Downloadordner.
        """
        try:
            print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
            print("Aufräumen...")
            filelist = [ file for file in os.listdir(csv_ziel) if file.endswith(".csv")]

            del_counter = 0

            if filelist:
                for file in filelist:
                    os.remove(os.path.join(csv_ziel, file))
                    del_counter += 1
                print(del_counter,"gelöschte Dateien")
                print("=================================================")

            else:
                print("Keine *.csv - Dateien zum löschen vorhanden!")
                
        except Exception as e:
            print("============================================================")
            print(time.strftime("%d.%m.%Y %H:%M:%S", time.localtime()))
            print("Fehler beim Löschen der csv-Dateien (*.csv):")
            print(e)
            print("============================================================")                
                
                
                
def rufe_Beobachtungsdaten_ab(ftp_server_name, ftp_server_usr, ftp_server_pwd, csv_quelle, csv_ziel, db_host, db_usr, db_pwd, db_name, db_port):

    """
        Ruft die deutschen Beobachtungsdaten ab.
    """               
    
    try:                  
        # Definiere einen Datensatz für die deutschen Beobachtungsdaten:                 
        Dataset = ObsDataset(ftp_server_name, ftp_server_usr, ftp_server_pwd, csv_quelle, csv_ziel, db_host, db_usr, db_pwd, db_name, db_port)

        message_sent = Dataset.send_startmessage()
        #==============================================================================
        if message_sent.get('success'):
            # Zeigt die Eigenschaften des Datenabrufs:
            properties_shown = Dataset.show_properties()
        else:
            raise Exception(message_sent.get("err"), message_sent.get("msg"))
        #==============================================================================            
        if properties_shown.get('success'):    
            # Lade die deutschen Stationskennziffern in der Tabelle "Stationskennziffern", aber nicht die in der "ignorelist"
            get_stations = Dataset.get_stations_from_DB()
        else:
            raise Exception(properties_shown.get("err"), properties_shown.get("msg"))
        #==============================================================================        
        if get_stations.get('success'):
            # Lade alle Stationskennziffern in der Tabelle "Ignorelist"
            get_stations_ignore = Dataset.get_stations_from_ignorelist()
        else:
            raise Exception(get_stations.get("err"), get_stations.get("msg"))
        #==============================================================================            
        if get_stations_ignore.get('success'):
            # Erstelle einen Dataframe, der eine Zusammenfassung über alle zum Download freigegebenen csv-Dateien bereitstellt.
            files_downloaded = Dataset.download_csvFiles()
        else:
            raise Exception(get_stations_ignore.get("err"), get_stations_ignore.get("msg"))
        #==============================================================================            
        if files_downloaded.get('success'):
            # Schreibe die Stundenwerte der csv-Dateien in die Datenbank:
            data_written = Dataset.schreibe_Stundenwerte_in_Datenbank()
        else:
            raise Exception(files_downloaded.get("err"), files_downloaded.get("msg"))
        #==============================================================================
        if data_written.get('success'):
            # Schreibe die Tageswerte der csv-Dateien in die Datenbank:
            daily_data_written = Dataset.schreibe_Tageswerte_in_Datenbank()
        else:
            raise Exception(data_written.get("err"), data_written.get("msg"))       
        #==============================================================================        
        if not daily_data_written.get('success'):
            raise Exception(daily_data_written.get("err"), daily_data_written.get("msg")) 
        #==============================================================================            
    except Exception as e:
        print("============================================================")
        print("Fehler beim Abruf der GER-Datensätze!")
        for err in e.args: print(err)
        print("============================================================")        
    
    finally:
        # Aufräumen:
        Dataset.clean()


###################################################################################

rufe_Beobachtungsdaten_ab(ftp_server_name, ftp_server_usr, ftp_server_pwd, csv_quelle, csv_ziel, db_host, db_usr, db_pwd, db_name, db_port)



































