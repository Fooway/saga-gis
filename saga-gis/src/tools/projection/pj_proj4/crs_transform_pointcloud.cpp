/**********************************************************
 * Version $Id$
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                     Tool Library                      //
//                   Projection_Proj4                    //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//              crs_transform_pointcloud.cpp             //
//                                                       //
//                 Copyright (C) 2014 by                 //
//                    Volker Wichmann                    //
//                      Olaf Conrad                      //
//                                                       //
//-------------------------------------------------------//
//                                                       //
// This file is part of 'SAGA - System for Automated     //
// Geoscientific Analyses'. SAGA is free software; you   //
// can redistribute it and/or modify it under the terms  //
// of the GNU General Public License as published by the //
// Free Software Foundation, either version 2 of the     //
// License, or (at your option) any later version.       //
//                                                       //
// SAGA is distributed in the hope that it will be       //
// useful, but WITHOUT ANY WARRANTY; without even the    //
// implied warranty of MERCHANTABILITY or FITNESS FOR A  //
// PARTICULAR PURPOSE. See the GNU General Public        //
// License for more details.                             //
//                                                       //
// You should have received a copy of the GNU General    //
// Public License along with this program; if not, see   //
// <http://www.gnu.org/licenses/>.                       //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//    e-mail:     wichmann@laserdata.at                  //
//                                                       //
//    contact:    Volker Wichmann                        //
//                LASERDATA GmbH                         //
//                Management and analysis of             //
//                laserscanning data                     //
//                Innsbruck, Austria                     //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "crs_transform_pointcloud.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CCRS_Transform_PointCloud::CCRS_Transform_PointCloud(bool bList)
{
	m_bList	= bList;

	//-----------------------------------------------------
	Set_Name		(CSG_String::Format("%s (%s)", _TL("Coordinate Transformation"),
		bList ? _TL("Point Cloud List") : _TL("Point Cloud")
	));

	Set_Author		("V. Wichmann, O. Conrad (c) 2014");

	Set_Description	(_TW(
		"Coordinate transformation for point clouds."
	));

	Set_Description	(Get_Description() + "\n" + CSG_CRSProjector::Get_Description());

	//-----------------------------------------------------
	if( m_bList )
	{
		Parameters.Add_PointCloud_List("",
			"SOURCE"	, _TL("Source"),
			_TL(""),
			PARAMETER_INPUT
		);

		Parameters.Add_PointCloud_List("",
			"TARGET"	, _TL("Target"),
			_TL(""),
			PARAMETER_OUTPUT_OPTIONAL
		);
	}

	//-----------------------------------------------------
	else
	{
		Parameters.Add_PointCloud("",
			"SOURCE"	, _TL("Source"),
			_TL(""),
			PARAMETER_INPUT
		);

		Parameters.Add_PointCloud("",
			"TARGET"	, _TL("Target"),
			_TL(""),
			PARAMETER_OUTPUT
		);
	}
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CCRS_Transform_PointCloud::On_Execute_Transformation(void)
{
	if( m_bList )
	{
		CSG_Parameter_PointCloud_List	*pSources, *pTargets;

		pSources	= Parameters("SOURCE")->asPointCloudList();
		pTargets	= Parameters("TARGET")->asPointCloudList();

		pTargets->Del_Items();

		for(int i=0; i<pSources->Get_Item_Count() && Process_Get_Okay(false); i++)
		{
			CSG_PointCloud	*pSource	= pSources->Get_PointCloud(i);
			CSG_PointCloud	*pTarget	= SG_Create_PointCloud(pSource);

			pTarget->Set_Name(CSG_String::Format(SG_T("%s_transformed"), pSource->Get_Name()));

			if( Transform(pSource, pTarget) )
			{
				pTargets->Add_Item(pTarget);
			}
			else
			{
				delete(pTarget);
			}
		}

		return( pTargets->Get_Item_Count() > 0 );
	}
	else
	{
		CSG_PointCloud	*pSource	= Parameters("SOURCE")->asPointCloud();
		CSG_PointCloud	*pTarget	= Parameters("TARGET")->asPointCloud();

		if( pSource == pTarget )
		{
			CSG_String	sName = pSource->Get_Name();

			pTarget	= SG_Create_PointCloud(pSource);

			if( Transform(pSource, pTarget) )
			{
				pSource->Assign(pTarget);

				pSource->Set_Name(CSG_String::Format(SG_T("%s_transformed"), sName.c_str()));

				return( true );
			}
			else
			{
				delete(pTarget);

				return( false );
			}
		}
		else
		{
			pTarget->Create(pSource);

			pTarget->Set_Name(CSG_String::Format(SG_T("%s_transformed"), pSource->Get_Name()));
		}

		return( Transform(pSource, pTarget) );
	}
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CCRS_Transform_PointCloud::Transform(CSG_PointCloud *pSource, CSG_PointCloud *pTarget)
{
	if( !pTarget || !pSource || !pSource->is_Valid() )
	{
		return( false );
	}

	if( !m_Projector.Set_Source(pSource->Get_Projection()) )
	{
		return( false );
	}

	int		nDropped	= 0;

	Process_Set_Text(CSG_String::Format(SG_T("%s: %s"), _TL("Processing"), pSource->Get_Name()));

	for(int iPoint=0; iPoint<pSource->Get_Point_Count() && Set_Progress(iPoint, pSource->Get_Point_Count()); iPoint++)
	{
		TSG_Point_Z		Point = pSource->Get_Point(iPoint);

		if( m_Projector.Get_Projection(Point.x, Point.y, Point.z) )
		{
			pTarget->Add_Point(Point.x, Point.y, Point.z);

			for(int iField=0; iField<pSource->Get_Attribute_Count(); iField++)
			{
				pTarget->Set_Attribute(iField, pSource->Get_Attribute(iPoint, iField));
			}
		}
		else
		{
			nDropped++;
		}
	}

	if( nDropped > 0 )
	{
		Message_Add(CSG_String::Format(SG_T("%s: %d %s"), pTarget->Get_Name(), nDropped, _TL("points have been dropped")));
	}

	pTarget->Get_Projection() = m_Projector.Get_Target();

	return( pTarget->Get_Count() > 0 );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
