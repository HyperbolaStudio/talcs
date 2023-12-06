/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
 *                                                                            *
 * This file is part of TALCS.                                                *
 *                                                                            *
 * TALCS is free software: you can redistribute it and/or modify it under the *
 * terms of the GNU Lesser General Public License as published by the Free    *
 * Software Foundation, either version 3 of the License, or (at your option)  *
 * any later version.                                                         *
 *                                                                            *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY   *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  *
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for    *
 * more details.                                                              *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with TALCS. If not, see <https://www.gnu.org/licenses/>.             *
 ******************************************************************************/

#ifndef TALCSSYNTHESISGLOBAL_H
#define TALCSSYNTHESISGLOBAL_H

#include <QtGlobal>

#ifndef TALCSSYNTHESIS_EXPORT
#  ifdef TALCSSYNTHESIS_STATIC
#    define TALCSSYNTHESIS_EXPORT
#  else
#    ifdef TALCSSYNTHESIS_LIBRARY
#      define TALCSSYNTHESIS_EXPORT Q_DECL_EXPORT
#    else
#      define TALCSSYNTHESIS_EXPORT Q_DECL_IMPORT
#    endif
#  endif
#endif

#endif // TALCSSYNTHESISGLOBAL_H
