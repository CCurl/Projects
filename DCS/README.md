Objects defined by DCS

From: https://www.digitalcombatsimulator.com/en/support/faq/1254/

Good site: https://wiki.hoggitworld.com/view/Simulator_Scripting_Engine_Documentation

Game objects

There are several types of objects which the mission designer has access to. Some of these objects consist of a single instance (singleton), while several types may have multiple instances.

env. The simulator environment. Singleton. Provides access to program environment.

timer. The model timer of the simulator. Singleton. Provides current model time and current mission time.

land. The terrain. Singleton. Contains several functions for terrain analyzing.

atmosphere. The atmosphere. Singleton. Wind an other parameters.

world. The game world. Singleton.

coalition. The game coalitions: red, blue and neutral. Provides access to properties of coalitions such as bullseye, reference points and services.

country. The game countries. Provides only country list.

trigger. The trigger system. Singleton. Provides access to some trigger actions, trigger zones and user flags.

coord. Provides access to functions that convert between LO, MGRS, and latitude/longitude coordinate systems.

radio. Provides access to game radio system.

missionCommands. Provides access to mission commands in "F10. Other" menu.

AI. Provides constants used in Controller functions.

Object. The base category for static objects and units. Object is a static rigid body that has position (coordinate and orientation), category, type and descriptor.

CoalitionObject. Objects that belongs to a coalition and country. Intermediate class. This is the base class for static objects, airbases and unit and weapon.

Airbase. Airdrome, helipad or ship that acts as a base for aircraft.

StaticObject. It's possible to create new static objects during the mission and destroy them.

Weapon. Weapon utilities.

Unit. Active game object: aircraft, vehicle or ship. A Unit has several properties: name, coalition, state, type, number (in group), health, velocity, etc.

Units are always stored in a group.

A Unit may or may not have its own "Controller" object. If the unit does not have its own controller, then the unit is always under control of its group's controller, and thus, it is not possible to control the unit independently from its group.

For example, each aircraft has its own controller. An airborne group has its own controller too. This makes it possible to assign a task to any unit in the group, or to the whole group at once.

Naval and ground units do not have a controller for each unit, and only have a group controller, so you can only set a task to a whole naval or ground group.

Group. A group of units (ground, airborne and naval). Provides access to a group's properties, and to the units that a group consists of.

Controller. An instance of A.I. for a single unit or of a whole group. The mission designer may set a task to a controller, order a controller to perform a command, or change AI behavior options. 