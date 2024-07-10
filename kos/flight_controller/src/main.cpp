#include "../include/mission.h"
#include "../../shared/include/initialization_interface.h"
#include "../../shared/include/ipc_messages_initialization.h"
#include "../../shared/include/ipc_messages_autopilot_connector.h"
#include "../../shared/include/ipc_messages_credential_manager.h"
#include "../../shared/include/ipc_messages_navigation_system.h"
#include "../../shared/include/ipc_messages_periphery_controller.h"
#include "../../shared/include/ipc_messages_server_connector.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define RETRY_DELAY_SEC 1
#define RETRY_REQUEST_DELAY_SEC 5
#define FLY_ACCEPT_PERIOD_US 500000

#define GPS_COEF 10000000.0
#define LINE_WIDTH 8.0
#define EATH_RADIUS 6371000.0
struct Coords
{
    double latitude, longitude, altitude;
    Coords() {
        latitude = -1;
        longitude = -1;
        altitude = -1;
    }
    Coords (double lat, double lon, double alt) {
        latitude = lat;
        longitude = lon;
        altitude = alt;
    }
};


extern MissionCommand* commands;

int sendSignedMessage(char* method, char* response, char* errorMessage, uint8_t delay) {
    char message[512] = {0};
    char signature[257] = {0};
    char request[1024] = {0};
    snprintf(message, 512, "%s?%s", method, BOARD_ID);

    while (!signMessage(message, signature)) {
        fprintf(stderr, "[%s] Warning: Failed to sign %s message at Credential Manager. Trying again in %ds\n", ENTITY_NAME, errorMessage, delay);
        sleep(delay);
    }
    snprintf(request, 1024, "%s&sig=0x%s", message, signature);

    while (!sendRequest(request, response)) {
        fprintf(stderr, "[%s] Warning: Failed to send %s request through Server Connector. Trying again in %ds\n", ENTITY_NAME, errorMessage, delay);
        sleep(delay);
    }

    uint8_t authenticity = 0;
    while (!checkSignature(response, authenticity) || !authenticity) {
        fprintf(stderr, "[%s] Warning: Failed to check signature of %s response received through Server Connector. Trying again in %ds\n", ENTITY_NAME, errorMessage, delay);
        sleep(delay);
    }

    return 1;
}
int getCoordsTransform(Coords& coord) {
    fprintf(stderr, "===============\n");
    int32_t lati, longi, alti;
    if (!getCoords(lati, longi, alti)) {
        fprintf(stderr, "Receive error");
        return 1;
    }
    else {
        coord.latitude = lati / GPS_COEF;
        coord.longitude = longi / GPS_COEF;
        coord.altitude = alti / 1000.0;
        fprintf(stderr, "Coords:\n%f deg. latitude\n%f deg. longitude\nHeight: %f m.\n", coord.latitude, coord.longitude, coord.altitude);
        return 0;
    }
}

double havDist(Coords coord1, Coords coord2) {
    double lat1, lat2, lon1, lon2;
    lat1 = coord1.latitude * M_PI / 180;
    lat2 = coord2.latitude * M_PI / 180;
    lon1 = coord1.longitude * M_PI / 180;
    lon2 = coord2.longitude * M_PI / 180;
    return 2*EATH_RADIUS*asin(sqrt(0.5*(1-cos(lat2-lat1)+cos(lat1)*cos(lat2)*(1-cos(lon2-lon1)))));
}

Coords normalCrossPoint (Coords wp1, Coords wp2, Coords curPt) {
    double A[2][2], B[2], det;
    double lat1, lat2, lat3, lon1, lon2, lon3;
    lat1 = wp1.latitude * M_PI / 180;
    lat2 = wp2.latitude * M_PI / 180;
    lat3 = curPt.latitude * M_PI / 180;
    lon1 = wp1.longitude * M_PI / 180;
    lon2 = wp2.longitude * M_PI / 180;
    lon3 = curPt.longitude * M_PI / 180;
    Coords retStruct;
    A[0][0] = (lat2 - lat1) / (lon2 - lon1);
    A[0][1] = -1;
    A[1][0] = (lon1 - lon2) / (lat2 - lat1);
    A[1][1] = -1;
    B[0] = lat1 - lon1 * (lat2 - lat1) / (lon2 - lon1);
    B[1] = lat3 - lon3 * (lon1 - lon2) / (lat2 - lat1);
    det = A[0][0] * A[1][1] - A[1][0] * A[0][1];
    retStruct.longitude = (B[0] * A[1][1] - B[1] * A[0][1]) * (-1) / det / M_PI * 180;
    retStruct.latitude = (B[1] * A[0][0] - B[0] * A[1][0]) * (-1) / det / M_PI * 180;
    fprintf(stderr, "la %f lo %f\n", retStruct.latitude, retStruct.longitude);
    return retStruct;
}

bool isOnTheWay(Coords prevWp, Coords nextWp, Coords curPt) {
    Coords ncp = normalCrossPoint(prevWp, nextWp, curPt);
    double hav;
    hav = havDist(curPt, ncp);
    fprintf(stderr, "hav = %f\n", hav);
    if (havDist(curPt, ncp) < LINE_WIDTH / 2)
        return 1;
    else
        return 0;
}

Coords cwpToCoords(CommandWaypoint cwp) {
    Coords retStruct;
    retStruct.altitude = cwp.altitude / 1000.0;
    retStruct.latitude = double(cwp.latitude) / GPS_COEF;
    retStruct.longitude = double(cwp.longitude) / GPS_COEF;
    fprintf(stderr, "al %f la %f lo %f\n", retStruct.altitude, retStruct.latitude, retStruct.longitude);
    return retStruct;
}

int main(void) {
    //Before do anything, we need to ensure, that other modules are ready to work
    while (!waitForInit("periphery_controller_connection", "PeripheryController")) {
        fprintf(stderr, "[%s] Warning: Failed to receive initialization notification from Periphery Controller. Trying again in %ds\n", ENTITY_NAME, RETRY_DELAY_SEC);
        sleep(RETRY_DELAY_SEC);
    }
    while (!waitForInit("autopilot_connector_connection", "AutopilotConnector")) {
        fprintf(stderr, "[%s] Warning: Failed to receive initialization notification from Autopilot Connector. Trying again in %ds\n", ENTITY_NAME, RETRY_DELAY_SEC);
        sleep(RETRY_DELAY_SEC);
    }
    while (!waitForInit("navigation_system_connection", "NavigationSystem")) {
        fprintf(stderr, "[%s] Warning: Failed to receive initialization notification from Navigation System. Trying again in %ds\n", ENTITY_NAME, RETRY_DELAY_SEC);
        sleep(RETRY_DELAY_SEC);
    }
    while (!waitForInit("server_connector_connection", "ServerConnector")) {
        fprintf(stderr, "[%s] Warning: Failed to receive initialization notification from Server Connector. Trying again in %ds\n", ENTITY_NAME, RETRY_DELAY_SEC);
        sleep(RETRY_DELAY_SEC);
    }
    while (!waitForInit("credential_manager_connection", "CredentialManager")) {
        fprintf(stderr, "[%s] Warning: Failed to receive initialization notification from Credential Manager. Trying again in %ds\n", ENTITY_NAME, RETRY_DELAY_SEC);
        sleep(RETRY_DELAY_SEC);
    }
    fprintf(stderr, "[%s] Info: Initialization is finished\n", ENTITY_NAME);

    //Enable buzzer to indicate, that all modules has been initialized
    if (!enableBuzzer())
        fprintf(stderr, "[%s] Warning: Failed to enable buzzer at Periphery Controller\n", ENTITY_NAME);

    //Copter need to be registered at ORVD
    char authResponse[1024] = {0};
    sendSignedMessage("/api/auth", authResponse, "authentication", RETRY_DELAY_SEC);
    fprintf(stderr, "[%s] Info: Successfully authenticated on the server\n", ENTITY_NAME);

    //Constantly ask server, if mission for the drone is available. Parse it and ensure, that mission is correct
    while (true) {
        char missionResponse[1024] = {0};
        if (sendSignedMessage("/api/fmission_kos", missionResponse, "mission", RETRY_DELAY_SEC) && parseMission(missionResponse)) {
            fprintf(stderr, "[%s] Info: Successfully received mission from the server\n", ENTITY_NAME);
            printMission();
            break;
        }
        sleep(RETRY_REQUEST_DELAY_SEC);
    }

    //The drone is ready to arm
    fprintf(stderr, "[%s] Info: Ready to arm\n", ENTITY_NAME);
    while (true) {
        //Wait, until autopilot wants to arm (and fails so, as motors are disabled by default)
        while (!waitForArmRequest()) {
            fprintf(stderr, "[%s] Warning: Failed to receive an arm request from Autopilot Connector. Trying again in %ds\n", ENTITY_NAME, RETRY_DELAY_SEC);
            sleep(RETRY_DELAY_SEC);
        }
        fprintf(stderr, "[%s] Info: Received arm request. Notifying the server\n", ENTITY_NAME);

        //When autopilot asked for arm, we need to receive permission from ORVD
        char armRespone[1024] = {0};
        sendSignedMessage("/api/arm", armRespone, "arm", RETRY_DELAY_SEC);

        if (strstr(armRespone, "$Arm: 0#") != NULL) {
            //If arm was permitted, we enable motors
            fprintf(stderr, "[%s] Info: Arm is permitted\n", ENTITY_NAME);
            while (!setKillSwitch(true)) {
                fprintf(stderr, "[%s] Warning: Failed to permit motor usage at Periphery Controller. Trying again in %ds\n", ENTITY_NAME, RETRY_DELAY_SEC);
                sleep(RETRY_DELAY_SEC);
            }
            if (!permitArm())
                fprintf(stderr, "[%s] Warning: Failed to permit arm through Autopilot Connector\n", ENTITY_NAME);
            break;
        }
        else if (strstr(armRespone, "$Arm: 1#") != NULL) {
            fprintf(stderr, "[%s] Info: Arm is forbidden\n", ENTITY_NAME);
            if (!forbidArm())
                fprintf(stderr, "[%s] Warning: Failed to forbid arm through Autopilot Connector\n", ENTITY_NAME);
        }
        else
            fprintf(stderr, "[%s] Warning: Failed to parse server response\n", ENTITY_NAME);
        fprintf(stderr, "[%s] Warning: Arm was not allowed. Waiting for another arm request from autopilot\n", ENTITY_NAME);
    };

    //If we get here, the drone is able to arm and start the mission
    //The flight is need to be controlled from now on
    //Also we need to check on ORVD, whether the flight is still allowed or it is need to be paused

    Coords curCoord;
    uint16_t prevWp, nextWp;
    Coords prevCoords, nextCoords;
    double hav;
    nextWp = 1;
    prevWp = 0;
    fprintf(stderr, "1\n");
    while (commands[nextWp].type != WAYPOINT) {
            fprintf(stderr, "wp type = %d\n", commands[nextWp].type);
            nextWp++;
    }
    fprintf(stderr, "2\n");
    while (nextWp < 19) {
        prevCoords = cwpToCoords(commands[prevWp].content.waypoint);
        nextCoords = cwpToCoords(commands[nextWp].content.waypoint);
        getCoordsTransform(curCoord);
        hav = havDist(curCoord, nextCoords);
        if (hav < 2)
            prevWp = nextWp++;
        while (commands[nextWp].type != WAYPOINT) {
            fprintf(stderr, "wp type = %d\n", commands[nextWp].type);
            nextWp++;
        }
        fprintf(stderr, "wp dist = %f\n", hav);
        if (isOnTheWay(prevCoords, nextCoords, curCoord))
            fprintf(stderr, "Inside\n");
        else {
            fprintf(stderr, "Outside\n");
            //resumeFlight();
        }
        fprintf(stderr, "prev = %d\nnext = %d\n", prevWp, nextWp);
        sleep(1);
    }

    return EXIT_SUCCESS;
}