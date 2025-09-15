
SRCDIR = ../source

GDAL_INCLUDE_DIR = /usr/include/gdal
HDF5_INCLUDE_DIR = /usr/include/hdf5/serial

HDF5_LIB_DIR = /usr/lib/x86_64-linux-gnu/hdf5/serial


#compiler settings
CC = gcc

CFLAGS   = -c -g -fopenmp -Wall -O2 -march=native -D BOOST_NO_CXX11_SCOPED_ENUMS -std=c++14 -I$(SRCDIR) -I$(GDAL_INCLUDE_DIR) -I$(HDF5_INCLUDE_DIR)

LDFLAGSA = -std=c++14 -I$(SRCDIR)
LDFLAGSB = -L$(HDF5_LIB_DIR) -lboost_thread -lboost_system -lboost_filesystem -lboost_program_options -lboost_date_time -lboost_chrono -lgdal -lnetcdf -fopenmp -lm -lstdc++ -lhdf5


SOURCE_FILES= \
$(SRCDIR)/acheminement_riviere.cpp \
$(SRCDIR)/barrage_historique.cpp \
$(SRCDIR)/bilan_vertical.cpp \
$(SRCDIR)/bv3c1.cpp \
$(SRCDIR)/bv3c2.cpp \
$(SRCDIR)/cequeau.cpp \
$(SRCDIR)/coordonnee.cpp \
$(SRCDIR)/correction.cpp \
$(SRCDIR)/corrections.cpp \
$(SRCDIR)/date_heure.cpp \
$(SRCDIR)/degre_jour_bande.cpp \
$(SRCDIR)/degre_jour_glacier.cpp \
$(SRCDIR)/degre_jour_modifie.cpp \
$(SRCDIR)/donnee_meteo.cpp \
$(SRCDIR)/erreur.cpp \
$(SRCDIR)/etp_mc_guiness.cpp \
$(SRCDIR)/evapotranspiration.cpp \
$(SRCDIR)/fonte_glacier.cpp \
$(SRCDIR)/fonte_neige.cpp \
$(SRCDIR)/gdal_util.cpp \
$(SRCDIR)/grille_meteo.cpp \
$(SRCDIR)/grille_neige.cpp \
$(SRCDIR)/grille_prevision.cpp \
$(SRCDIR)/groupe_zone.cpp \
$(SRCDIR)/hydrotel.cpp \
$(SRCDIR)/hydro_quebec.cpp \
$(SRCDIR)/interpolation_donnees.cpp \
$(SRCDIR)/lac.cpp \
$(SRCDIR)/lac_sans_laminage.cpp \
$(SRCDIR)/lecture_acheminement_riviere.cpp \
$(SRCDIR)/lecture_bilan_vertical.cpp \
$(SRCDIR)/lecture_evapotranspiration.cpp \
$(SRCDIR)/lecture_fonte_glacier.cpp \
$(SRCDIR)/lecture_fonte_neige.cpp \
$(SRCDIR)/lecture_interpolation_donnees.cpp \
$(SRCDIR)/lecture_ruisselement_surface.cpp \
$(SRCDIR)/lecture_tempsol.cpp \
$(SRCDIR)/linacre.cpp \
$(SRCDIR)/log-performance.cpp \
$(SRCDIR)/milieu_humide_isole.cpp \
$(SRCDIR)/milieu_humide_riverain.cpp \
$(SRCDIR)/mise_a_jour.cpp \
$(SRCDIR)/moyenne_3_stations1.cpp \
$(SRCDIR)/moyenne_3_stations2.cpp \
$(SRCDIR)/noeud.cpp \
$(SRCDIR)/noeuds.cpp \
$(SRCDIR)/occupation_sol.cpp \
$(SRCDIR)/onde_cinematique.cpp \
$(SRCDIR)/onde_cinematique_modifiee.cpp \
$(SRCDIR)/output.cpp \
$(SRCDIR)/penman.cpp \
$(SRCDIR)/penman_monteith.cpp \
$(SRCDIR)/prelevements.cpp \
$(SRCDIR)/prelevements_donnees.cpp \
$(SRCDIR)/prelevements_site.cpp \
$(SRCDIR)/priestlay_taylor.cpp \
$(SRCDIR)/projection.cpp \
$(SRCDIR)/propriete_hydrolique.cpp \
$(SRCDIR)/propriete_hydroliques.cpp \
$(SRCDIR)/rankinen.cpp \
$(SRCDIR)/raster_double2.cpp \
$(SRCDIR)/raster_float2.cpp \
$(SRCDIR)/raster_int2.cpp \
$(SRCDIR)/rayonnement_net.cpp \
$(SRCDIR)/riviere.cpp \
$(SRCDIR)/ruisselement_surface.cpp \
$(SRCDIR)/sim_hyd.cpp \
$(SRCDIR)/sous_modele.cpp \
$(SRCDIR)/station.cpp \
$(SRCDIR)/stations.cpp \
$(SRCDIR)/stations_hydro.cpp \
$(SRCDIR)/stations_meteo.cpp \
$(SRCDIR)/stations_neige.cpp \
$(SRCDIR)/station_hydro.cpp \
$(SRCDIR)/station_hydro_gibsi.cpp \
$(SRCDIR)/station_meteo.cpp \
$(SRCDIR)/station_meteo_gibsi.cpp \
$(SRCDIR)/station_meteo_hdf5.cpp \
$(SRCDIR)/station_meteo_netcdf.cpp \
$(SRCDIR)/station_meteo_netcdf_station.cpp \
$(SRCDIR)/station_neige.cpp \
$(SRCDIR)/station_neige_mef.cpp \
$(SRCDIR)/statistiques.cpp \
$(SRCDIR)/tempsol.cpp \
$(SRCDIR)/thiessen1.cpp \
$(SRCDIR)/thiessen2.cpp \
$(SRCDIR)/thornthwaite.cpp \
$(SRCDIR)/thorsen.cpp \
$(SRCDIR)/transforme_coordonnee.cpp \
$(SRCDIR)/troncon.cpp \
$(SRCDIR)/troncons.cpp \
$(SRCDIR)/util.cpp \
$(SRCDIR)/zone.cpp \
$(SRCDIR)/zones.cpp \


HEADER_FILES= \
$(SRCDIR)/acheminement_riviere.hpp \
$(SRCDIR)/barrage_historique.hpp \
$(SRCDIR)/bilan_vertical.hpp \
$(SRCDIR)/bv3c1.hpp \
$(SRCDIR)/bv3c2.hpp \
$(SRCDIR)/cequeau.hpp \
$(SRCDIR)/constantes.hpp \
$(SRCDIR)/coordonnee.hpp \
$(SRCDIR)/correction.hpp \
$(SRCDIR)/corrections.hpp \
$(SRCDIR)/date_heure.hpp \
$(SRCDIR)/degre_jour_bande.hpp \
$(SRCDIR)/degre_jour_glacier.hpp \
$(SRCDIR)/degre_jour_modifie.hpp \
$(SRCDIR)/donnee_meteo.hpp \
$(SRCDIR)/erreur.hpp \
$(SRCDIR)/etp_mc_guiness.hpp \
$(SRCDIR)/evapotranspiration.hpp \
$(SRCDIR)/fonte_glacier.hpp \
$(SRCDIR)/fonte_neige.hpp \
$(SRCDIR)/gdal_util.hpp \
$(SRCDIR)/grille_meteo.hpp \
$(SRCDIR)/grille_neige.hpp \
$(SRCDIR)/grille_prevision.hpp \
$(SRCDIR)/groupe_zone.hpp \
$(SRCDIR)/hydro_quebec.hpp \
$(SRCDIR)/interpolation_donnees.hpp \
$(SRCDIR)/lac.hpp \
$(SRCDIR)/lac_sans_laminage.hpp \
$(SRCDIR)/lecture_acheminement_riviere.hpp \
$(SRCDIR)/lecture_bilan_vertical.hpp \
$(SRCDIR)/lecture_evapotranspiration.hpp \
$(SRCDIR)/lecture_fonte_glacier.hpp \
$(SRCDIR)/lecture_fonte_neige.hpp \
$(SRCDIR)/lecture_interpolation_donnees.hpp \
$(SRCDIR)/lecture_ruisselement_surface.hpp \
$(SRCDIR)/lecture_tempsol.hpp \
$(SRCDIR)/linacre.hpp \
$(SRCDIR)/log-performance.hpp \
$(SRCDIR)/matrice.hpp \
$(SRCDIR)/milieu_humide_isole.hpp \
$(SRCDIR)/milieu_humide_riverain.hpp \
$(SRCDIR)/mise_a_jour.hpp \
$(SRCDIR)/moyenne_3_stations1.hpp \
$(SRCDIR)/moyenne_3_stations2.hpp \
$(SRCDIR)/noeud.hpp \
$(SRCDIR)/noeuds.hpp \
$(SRCDIR)/occupation_sol.hpp \
$(SRCDIR)/onde_cinematique.hpp \
$(SRCDIR)/onde_cinematique_modifiee.hpp \
$(SRCDIR)/output.hpp \
$(SRCDIR)/penman.hpp \
$(SRCDIR)/penman_monteith.hpp \
$(SRCDIR)/point.hpp \
$(SRCDIR)/prelevements.hpp \
$(SRCDIR)/prelevements_donnees.hpp \
$(SRCDIR)/prelevements_site.hpp \
$(SRCDIR)/priestlay_taylor.hpp \
$(SRCDIR)/projection.hpp \
$(SRCDIR)/projections.hpp \
$(SRCDIR)/propriete_hydrolique.hpp \
$(SRCDIR)/propriete_hydroliques.hpp \
$(SRCDIR)/rankinen.hpp \
$(SRCDIR)/raster.hpp \
$(SRCDIR)/raster_double2.hpp \
$(SRCDIR)/raster_float2.hpp \
$(SRCDIR)/raster_int2.hpp \
$(SRCDIR)/rayonnement_net.hpp \
$(SRCDIR)/riviere.hpp \
$(SRCDIR)/ruisselement_surface.hpp \
$(SRCDIR)/sim_hyd.hpp \
$(SRCDIR)/sous_modele.hpp \
$(SRCDIR)/station.hpp \
$(SRCDIR)/stations.hpp \
$(SRCDIR)/stations_hydro.hpp \
$(SRCDIR)/stations_meteo.hpp \
$(SRCDIR)/stations_neige.hpp \
$(SRCDIR)/station_hydro.hpp \
$(SRCDIR)/station_hydro_gibsi.hpp \
$(SRCDIR)/station_meteo.hpp \
$(SRCDIR)/station_meteo_gibsi.hpp \
$(SRCDIR)/station_meteo_hdf5.hpp \
$(SRCDIR)/station_meteo_netcdf.hpp \
$(SRCDIR)/station_meteo_netcdf_station.hpp \
$(SRCDIR)/station_neige.hpp \
$(SRCDIR)/station_neige_mef.hpp \
$(SRCDIR)/statistiques.hpp \
$(SRCDIR)/tempsol.hpp \
$(SRCDIR)/thiessen1.hpp \
$(SRCDIR)/thiessen2.hpp \
$(SRCDIR)/thornthwaite.hpp \
$(SRCDIR)/thorsen.hpp \
$(SRCDIR)/transforme_coordonnee.hpp \
$(SRCDIR)/troncon.hpp \
$(SRCDIR)/troncons.hpp \
$(SRCDIR)/types.hpp \
$(SRCDIR)/util.hpp \
$(SRCDIR)/version.hpp \
$(SRCDIR)/zone.hpp \
$(SRCDIR)/zones.hpp


SOURCES=$(SOURCE_FILES) $(HEADER_FILES) 
OBJECTS=$(SOURCE_FILES:.cpp=.o)
EXECUTABLE=hydrotel

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGSA) $(LDFLAGSB)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(SRCDIR)/*.o
	rm -f hydrotel
	
