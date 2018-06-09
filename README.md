# ZAP
MS DOS Game developed in 1995 with Borland CPP and Assembly for the class Programação Orientada por Objectos of the Master in Electronic and Telecomunications Engineering from the University of Aveiro

Description
ZAP is an arcade game developed in Borland C++ 3.1, and runs under DOS.
It is written in an object-oriented language, which is very useful in programs like this one, because it helps the organization and control of the various objects of the game and the interaction with the user.
The game is a very simple one, with a hero that has to kill all the enemies and catch as much bonus points as he can. There are some kind of objects that one can throw against the evil snakes, but their number is limited. The snakes move in a predefined way and when they touch the hero he loses energy. When the energy is over he loses one life.
The environment is a garden in a forest and the hero has to jump over the platforms and avoid the obstacles to reach his objective. The background scrolls in two dimensions all over the map of the game, following the hero.
In this brief introduction can be seen that there are lots of entities that we can assign to objects in C++ in a natural way:
 	the evil snakes,
 	the objects that give points,
 	the ammunition,
 	the background,
Those are the main objects of the game, but the keyword reading, graphics dealing, palette setting and music are also very important.
To understand the class hierarchy there is a diagram in the next page that shows the relationships between the base classes and derived classes. The remaining classes are not represented there because they are very simple in terms of block diagram and they have nothing relevant to add to those of the diagram.
The diagram has the complete definition of the classes, and arrows indicating the base class, where we can perfectly see the encapsulation and inheritance.
The code of the program is divided in three modules:
  The module MYCLASS.CPP has got all the declaration of the variables, functions and classes.
 	The module MYCLASS.CPP has the implementation of the functions declared in the header file.
 	The module GAME.CPP has got the main function and template declaration and implementation. In this module we can control the game in terms of order of appearing a chosen object in the screen, the speed of the game, what graphics are loaded in memory, and so on. Changing the order of some instructions has interesting effects, like making the hero travel behind the background.
Because this program is quite big it's a hard task to go through the code and understand everything. There are also lots of functions written in assembly line but the code is full commented making it easier to follow and understand.

Copyright (C) 1995 by Jose Fonseca (josefonseca at ipg dot pt , jozefonseca at gmail dot com)
