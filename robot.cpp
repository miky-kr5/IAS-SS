#include "robot.hpp"

Robot::Robot(std::string hostname, uint32_t port) {
  _p_client = new PlayerCc::PlayerClient(hostname, port);
  _p_proxy = new PlayerCc::Position2dProxy(_p_client, 0);
}

Robot::~Robot() {
  delete _p_proxy;
  delete _p_client;
}

IASSS_Robot::IASSS_Robot(std::string hostname, uint32_t port) : Robot(hostname, port) {
  std::cout << "Creating IAS-SS robot on host \"" << hostname << "\" and port " << port << "." << std::endl;
}

IASSS_Robot::~IASSS_Robot() {
  std::cout << "Destroying IAS-SS robot." << std::endl;
}

void IASSS_Robot::run() {
  sleep(10);
}
