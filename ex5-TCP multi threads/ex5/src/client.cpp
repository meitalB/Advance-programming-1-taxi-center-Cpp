
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include "persons/Driver.h"
#include "creators/DriverCreator.h"
#include "Serialization.h"
#include "sockets/TcpClient.h"

//insert a driver as a client
Driver* insertDriver() {

    char dummy;
    int id, age;
    char status;
    int experience, vehicleId;
    std::cin >> id >> dummy >> age >> dummy >> status >> dummy >> experience >> dummy >> vehicleId;
    Driver* d = createDriver(id, age, status, experience, vehicleId);
    return d;

}


int main(int argc, char *argv[]) {
    char buffer[120000];
    Ride *ride = NULL;
    Trip *t = NULL;
    long readBytes;


    TcpClient* tcpClient = new TcpClient(argv[1], atoi(argv[2]));
    tcpClient->Connect();

    std::cout<<"start serialize \n";
    //create a driver
    Driver* driver = insertDriver();
    //serialize driver
    std::string serial_str_driver = serialize<Driver>(driver);
    tcpClient->sendData(serial_str_driver);
    std::cout<<"finish serialize driver\n";

    readBytes = tcpClient->receiveData(buffer, sizeof(buffer));
    std::string vehicleStr(buffer, readBytes);
    //deserialize receive vehicle
    TaxiCab *taxiCab = deserialize<TaxiCab>(vehicleStr);
    driver->setTaxiCab(taxiCab);

    tcpClient->receiveData(buffer, sizeof(buffer));
    char option = buffer[0];
    std::string data;
    while (option != 'E') {
        switch (option) {
            case 'T':
                readBytes = tcpClient->receiveData(buffer, sizeof(buffer));
                data = string(buffer, readBytes);
                if (ride == NULL) {
                    t = deserialize<Trip>(data);
                    ride = new Ride(t, driver);
                }
                break;

            case 'G':
                if (ride != NULL) {
                    ride->moveOneStep();
                    if (ride->isDone()) {
                        delete ride;
                        ride = NULL;
                    }
                }
                break;

            default:
                break;
        }
        tcpClient->receiveData(buffer, sizeof(buffer));
        option = buffer[0];
    }

    delete tcpClient;
    delete driver;
    if(ride != NULL) {
        delete ride;
    }
    return 0;
}

