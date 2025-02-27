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

#ifndef MATRICE_H_INCLUDED
#define MATRICE_H_INCLUDED

#include <cstddef>
#include <vector>

#include <boost/assert.hpp>


namespace HYDROTEL 
{

	template<typename T>
	class MATRICE
	{
	public:
		MATRICE();
		MATRICE(const MATRICE<T>& m);

		/// move constructor
		MATRICE(MATRICE<T>&& m);

		MATRICE(size_t nb_ligne, size_t nb_colonne, T v = T());

		~MATRICE();

		/// operator =
		MATRICE<T>& operator= (const MATRICE<T>& m);
		
		size_t PrendreNbLigne() const;

		size_t PrendreNbColonne() const;

		/// retourne la valeur
		T& operator () (size_t ligne, size_t colonne);

		/// retourne la valeur
		const T& operator () (size_t ligne, size_t colonne) const;

		/// retourne un pointeur sur les valeurs
		T* PrendrePtr();

		/// echange les donnees avec une autre MATRICE
		void swap(MATRICE<T>& m);

	private:
		size_t _nb_ligne;
		size_t _nb_colonne;
		std::vector<T> _donnee;
	};

	template<typename T>
	MATRICE<T>::MATRICE()
		: _nb_ligne(0)
		, _nb_colonne(0)
		, _donnee()
	{
	}

	template<typename T>
	MATRICE<T>::MATRICE(const MATRICE<T>& m)
		: _nb_ligne(m._nb_ligne)
		, _nb_colonne(m._nb_colonne)
		, _donnee(m._donnee)
	{
	}

	template<typename T>
	MATRICE<T>::MATRICE(MATRICE<T>&& m)
		: _nb_ligne(0)
		, _nb_colonne(0)
		, _donnee()
	{
		swap(m);
	}

	template<typename T>
	MATRICE<T>::MATRICE(size_t nb_ligne, size_t nb_colonne, T v)
		: _nb_ligne(nb_ligne)
		, _nb_colonne(nb_colonne)
		, _donnee(nb_ligne * nb_colonne, v)
	{
	}

	template<typename T>
	MATRICE<T>::~MATRICE()
	{
	}

	template<typename T>
	MATRICE<T>& MATRICE<T>::operator= (const MATRICE<T>& m)
	{
		_nb_ligne = m._nb_ligne;
		_nb_colonne = m._nb_colonne;
		_donnee = m._donnee;
		return *this;
	}

	template<typename T>
	size_t MATRICE<T>::PrendreNbLigne() const
	{
		return _nb_ligne;
	}

	template<typename T>
	size_t MATRICE<T>::PrendreNbColonne() const
	{
		return _nb_colonne;
	}

	template<typename T>
	T& MATRICE<T>::operator() (size_t ligne, size_t colonne)
	{
		BOOST_ASSERT(ligne < _nb_ligne && colonne < _nb_colonne);
		return _donnee[ligne * _nb_colonne + colonne];
	}

	template<typename T>
	const T& MATRICE<T>::operator() (size_t ligne, size_t colonne) const
	{
		BOOST_ASSERT(ligne < _nb_ligne && colonne < _nb_colonne);
		return _donnee[ligne * _nb_colonne + colonne];
	}

	template<typename T>
	T* MATRICE<T>::PrendrePtr()
	{
		return &_donnee[0];
	}

	template<typename T>
	void MATRICE<T>::swap(MATRICE<T>& m)
	{
		std::swap(_nb_ligne, m._nb_ligne);
		std::swap(_nb_colonne, m._nb_colonne);
		_donnee.swap(m._donnee);
	}

}

#endif
