/*
 * Copyright 2011 Scott MacDonald
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "gameclient.h"
#include <common/assert.h>
#include <common/delete.h>
#include <common/time.h>
#include <app/logging.h>
#include <app/platform.h>
#include "engine/gametime.h"
#include "graphics/iwindow.h"
#include "graphics/irenderer.h"

/**
 * Inspiration and help for the game loop came from the following sources:
 *
 * Gaffer: "Fix Your Timestep!"
 * http://gafferongames.com/game-physics/fix-your-timestep/
 *
 * Stack Exchange "Fixed time step vs variable time step"
 * http://gamedev.stackexchange.com/questions/1589/fixed-time-step-vs-variable-time-step
 *
 * "Understanding GameTime"
 * http://blogs.msdn.com/b/shawnhar/archive/2007/07/25/understanding-gametime.aspx
 *
 * Information about the granularity and precision of the Windows Timers
 * came from the following sources:
 * "http://www.geisswerks.com/ryan/FAQS/timing.html"
 */

/**
 * Game client constructor
 */
GameClient::GameClient( IWindow *pMainWindow,
                        IRenderer *pRenderer )
    : mpMainWindow( pMainWindow ),
      mpRenderer( NULL ),
      mIsGameRunning( false ),
      mIsRunningSlowly( false ),
      mUpdateFrequency( 1.0f / 50.0f ), // 20ms, 50 times per second
      mMaximumSleepSkew( 0.01f )        // 10ms
{
    assert( pMainWindow != NULL );
    assert( pRenderer != NULL );
}

/**
 * Game client destructor
 */
GameClient::~GameClient()
{
}

/**
 * Sets the number of times the game will call the update method
 */
void GameClient::setUpdateFrequency( int numUpdatesPerSecond )
{
    assert( numUpdatesPerSecond > 0 );

    // Calculate the new update frequency and verify that it isn't zero
    // for some ungodly reason
    mUpdateFrequency = Time( 1.0 / static_cast<double>(numUpdatesPerSecond) );

    // Logging
    LOG_TRACE("GameClient") << "Setting the update frequency to " << mUpdateFrequency;
}

/**
 * Starts up and runs the game. This method will not return until after the
 * player has quit the game
 */
int GameClient::run()
{
    // Show the main window before we set up our rendering system or load
    // any resources
    mpMainWindow->show();

    // Let the game initialize core systems
    if ( initializeClient() == false || initialize() == false )
    {
        return App::EPROGRAM_FATAL_ERROR;
    }

    // Now load resources before entering the main game loop
    if (! loadContent() )
    {
        return App::EPROGRAM_FATAL_ERROR;
    }

    // Enter the game
    runMainGameLoop();

    // Make sure we unload all of our game's resources before the game client
    // exits
    unloadContent();
    return App::EPROGRAM_OK;
}

/**
 * Core game loop logic
 */
int GameClient::runMainGameLoop()
{
    LOG_INFO("GameClient") << "Entering the main game loop";
    mIsGameRunning = true;

    // Start simulation time tracking
    Time simulationTime  = Time( 0.0 );   // t
    Time accumulatedTime = Time( 0.0 );
    Time systemTime      = App::currentTime();   // currentTime

    // This is where it all starts
    while ( mIsGameRunning && (!mpMainWindow->didUserQuit()) )
    {
        // Make sure we process ALL THE MESSAGES (before doing any useful
        // simulation stuffs)
        mpMainWindow->processMessages();

        // Get the current system time, and then calculate how much time
        // has elapsed since the last graphics update (which we will call
        // frameTime)
        Time newTime   = App::currentTime();
        Time frameTime = newTime - systemTime;

        systemTime = newTime;

        // Check the amount of time that we have spent since the last loop
        // iteration. If the value exceeds a threshold, assume that we are in
        // danger of hitting the "spiral of death" from a slow simulator. To
        // avoid this, limit the maximum frame time to a more reasonable value
        if ( frameTime > Time(0.25) )
        {
            frameTime = Time(0.25);
        }

        // Update the simulation. If the simulation is running too far behind
        // the system time we will need to run the simulation multiple times
        // until it is caught up
        size_t numUpdates  = 0;
        accumulatedTime   += frameTime;

        while ( accumulatedTime >= mUpdateFrequency )
        {
            // Is the simulation running multiple times?
            //   (What's the best way to tell if simulation is slow or gfx?)
            mIsRunningSlowly = ( numUpdates > 0 );

            // Update the simulation
            update( simulationTime, mUpdateFrequency );

            // The simulation has been updated, and we need to increment our
            // time tracking variables before the next (possible) loop iteration
            numUpdates      += 1;
            simulationTime  += mUpdateFrequency;
            accumulatedTime -= mUpdateFrequency;
        }

        // Calculate the amount of interpolation that will our renderer will
        // need to account for when rendering between the last simulation update
        // and the next upcoming update
        float interpolation =
            1.0f - ( accumulatedTime.toDouble() / mUpdateFrequency.toDouble() );

        // Now draw the next frame
        draw( simulationTime, interpolation );
        mpRenderer->present();

        // If there is a large delta between the system time and the time before
        // the next simulation update, we can afford to sleep a tiny bit and
        // allow windows some breathing room. (This can be tweaked or disabled)
        if ( accumulatedTime + mMaximumSleepSkew < mUpdateFrequency )
        {
            App::sleep( Time( 2.0 ) );
        }
    }

    LOG_NOTICE("GameClient") << "Game has left the main game loop";
    return App::EPROGRAM_OK;
}

/**
 * Code that initializes the game client and all required game subsystems
 * such as rendering, resource management and sound.
 *
 * \return  True if all components were initialized successfully, false otherwies
 */
bool GameClient::initializeClient()
{
    LOG_DEBUG("GameClient") << "Initializing the game client";
    return true;
}

/**
 * Initializes the game client in preparation for the running the game. This method
 * should be overridden by an implementing class, and should load any classes or
 * systems that are required for first play.
 *
 * \return  True if all components were initialized successfully, false otherwise
 */
bool GameClient::initialize()
{
    return true;
}

/**
 * Loads content required for the game to function
 */
bool GameClient::loadContent()
{
    LOG_NOTICE("GameClient") << "Loading the game's art assets";
    return true;
}

void GameClient::unloadContent()
{
    LOG_NOTICE("GameClient") << "Unloading the game's art assets";
}

/**
 * This is an override able method that is called by the main game loop and is
 * supposed to be advance the simulation by deltaTime.
 *
 * The simulation time is divorced from the system time, and is stepped by a
 * constant amount every call. It may be called multiple times in a row to ensure
 * that the simulation is properly caught up.
 *
 * \param  simlulationTime  The current simulation time
 * \param  deltaTime        Amount of time that has elapsed since the last call
 *                          to this method (Always the same amount)
 */
void GameClient::update( Time /*simulationTime*/, Time /*deltaTime*/ )
{
    // empty for now
}

/**
 * Draw is called by the main game loop to render the simulation display. It
 * should be overridden by the implementing class to implement game specific drawing
 * code, seeing as the default implementation does nothing. :)
 *
 * \param  simulationTime  The current simulation time
 * \param  deltaTime       Amount of time since the last simulation
 * \param  interpolation   Amount to interpolate between (1.0 use the simT)
 */
void GameClient::draw( Time /*simulationTime*/, float /*interpolation*/ )
{
    // empty for now
}
