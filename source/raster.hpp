//
// HYDROTEL a spatially distributed hydrological model
// Copyright (C) 2013 INRS Eau Terre Environnement
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
// USA
//

#ifndef RASTER_H_INCLUDED
#define RASTER_H_INCLUDED


#include "coordonnee.hpp"
#include "matrice.hpp"
#include "projection.hpp"

#include <boost/assert.hpp>


namespace HYDROTEL
{

	template <typename T>
	class RASTER
	{
	public:
		RASTER(COORDONNEE c = COORDONNEE(), PROJECTION p = PROJECTION(), size_t nb_ligne = 0, size_t nb_colonne = 0, float xresolution = 1.0, float yresolution = 1.0, T nodata = T());
		RASTER(const RASTER<T>& r);

		// move constructor
		RASTER(RASTER<T>&& r);

		~RASTER();

		// operator =
		RASTER<T>& operator=(const RASTER<T>& r);
		
		// return map upper left corner
		const COORDONNEE& PrendreCoordonnee() const;

		const PROJECTION& PrendreProjection() const;

		size_t PrendreNbLigne() const;

		size_t PrendreNbColonne() const;

		// retourne la taille des cellules en x (m)
		float PrendreTailleCelluleX() const;

		// retourne la taille des cellules en y (m)
		float PrendreTailleCelluleY() const;

		// retourne la valeur
		T& operator () (size_t ligne, size_t colonne);

		// retourne la valeur
		const T& operator () (size_t ligne, size_t colonne) const;

		// retourne un pointeur sur les valeurs
		T* PrendrePtr();

		void CoordonneeVersLigCol(const COORDONNEE& coordonnee, int& ligne, int& colonne) const;

		COORDONNEE LigColVersCoordonnee(int ligne, int colonne) const;

		// retourne la valeur pour le nodata
		T PrendreNoData() const;

	private:
		COORDONNEE	_coordonnee;	// map upper left corner
		PROJECTION	_projection;	// projection
		float		_tailleX;       // m
		float		_tailleY;       // m
		T			_nodata;		// valeur des nodata

		MATRICE<T>	_donnee;		// donnee
	};


	template<typename T>
	RASTER<T>::RASTER(RASTER<T>&& r)
		: _coordonnee(r._coordonnee)
		, _projection(r._projection)
		, _tailleX(r._tailleX)
		, _tailleY(r._tailleY)
		, _nodata(r._nodata)
	{
		_projection = r._projection;
		_donnee.swap(r._donnee);
	}


	template<typename T>
	RASTER<T>::RASTER(COORDONNEE c, PROJECTION p, size_t nb_ligne, size_t nb_colonne, float tailleX, float tailleY, T nodata)
		: _coordonnee(c)
		, _projection(p)
		, _tailleX(tailleX)
		, _tailleY(tailleY)
		, _nodata(nodata)
		, _donnee(nb_ligne, nb_colonne, nodata)
	{
		BOOST_ASSERT(tailleX != 0 && tailleX == tailleY);

		_projection = p;
	}


	template<typename T>
	RASTER<T>::RASTER(const RASTER<T>& r)
		: _coordonnee(r._coordonnee)
		, _projection(r._projection)
		, _tailleX(r._tailleX)
		, _tailleY(r._tailleY)
		, _nodata(r._nodata)
		, _donnee(r._donnee)
	{
		_projection = r.PrendreProjection();
	}


	template<typename T>
	RASTER<T>::~RASTER()
	{
	}


	template<typename T>
	RASTER<T>& RASTER<T>::operator=(const RASTER<T>& r)
	{
		_coordonnee = r._coordonnee;
		_projection = r._projection;
		_tailleX = r._tailleX;
		_tailleY = r._tailleY;
		_nodata = r._nodata;
		_donnee = r._donnee;

		return *this;
	}


	template<typename T>
	const COORDONNEE& RASTER<T>::PrendreCoordonnee() const
	{
		return _coordonnee;
	}


	template<typename T>
	const PROJECTION& RASTER<T>::PrendreProjection() const
	{
		return _projection;
	}


	template<typename T>
	size_t RASTER<T>::PrendreNbLigne() const
	{
		return _donnee.PrendreNbLigne();
	}


	template<typename T>
	size_t RASTER<T>::PrendreNbColonne() const
	{
		return _donnee.PrendreNbColonne();
	}


	template<typename T>
	float RASTER<T>::PrendreTailleCelluleX() const
	{
		return _tailleX;
	}


	template<typename T>
	float RASTER<T>::PrendreTailleCelluleY() const
	{
		return _tailleY;
	}


	template<typename T>
	T RASTER<T>::PrendreNoData() const
	{
		return _nodata;
	}


	template<typename T>
	T& RASTER<T>::operator() (size_t ligne, size_t colonne)
	{
		BOOST_ASSERT(ligne < _donnee.PrendreNbLigne() && colonne < _donnee.PrendreNbColonne());
		return _donnee(ligne, colonne);
	}


	template<typename T>
	const T& RASTER<T>::operator() (size_t ligne, size_t colonne) const
	{
		BOOST_ASSERT(ligne < _donnee.PrendreNbLigne() && colonne < _donnee.PrendreNbColonne());
		return _donnee(ligne, colonne);
	}


	template<typename T>
	T* RASTER<T>::PrendrePtr()
	{
		return _donnee.PrendrePtr();
	}


	template<typename T>
	void RASTER<T>::CoordonneeVersLigCol(const COORDONNEE& coord, int& ligne, int& colonne) const
	{
		ligne   = static_cast<int>((_coordonnee.PrendreY() - coord.PrendreY()) / _tailleY);
		colonne = static_cast<int>((coord.PrendreX() - _coordonnee.PrendreX()) / _tailleX);
	}


	template<typename T>
	COORDONNEE RASTER<T>::LigColVersCoordonnee(int ligne, int colonne) const
	{
		//[ligne, colonne] relative to upper left corner of map. [0, 0] is upper left corner of map.
		double y = _coordonnee.PrendreY() - (ligne * _tailleY);
		double x = _coordonnee.PrendreX() + (colonne * _tailleX);

		return COORDONNEE(x, y);	//coord of upper left corner of pixel
	}

}

#endif

