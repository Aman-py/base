/*
 *  Copyright (C) 1997-2015 JDE Developers Team
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see http://www.gnu.org/licenses/.
 *  Authors :
 *       Victor Arribas Raigadas <v.arribas.urjc@gmai.com>
 */


#ifndef CMDVELI_H
#define CMDVELI_H


#include <jderobot/cmdvel.h>
#include <quadrotor/quadrotorcontrol.hh>


namespace quadrotor{
namespace interfaces{

class CMDVelI : public jderobot::CMDVel {
public:
    CMDVelI (quadrotor::QuadrotorControl *control);
    virtual ~CMDVelI ();

    Ice::Int setCMDVelData(const jderobot::CMDVelDataPtr&,
                           const Ice::Current&);

private:
    quadrotor::QuadrotorControl* const control;
};


}}//NS
#endif // CMDVELI_H
