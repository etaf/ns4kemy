#ifndef UTILITY_HH
#define UTILITY_HH
#include<vector>
#include<cmath>
#include<string>
class Utility
{
    private:
        std::vector<double> us; //utility parts
        const static std::vector<double> ws; //weights
    public:
        Utility():us(0){};
        Utility(std::vector<double> us_t):us(us_t){};
        bool improved (const Utility& other)const{
            double res = 0;
            for(unsigned int i=0;i<ws.size();++i){
                res += ws[i] * (other.us[i] - us[i]) / fabs(us[i] + other.us[i]);
            }
            return res > 0;
        }

        void operator += (const Utility& other){
            if(us.empty()){
                us = other.us;
                return;
            }
            for(unsigned int i=0;i < ws.size(); ++i){
                us[i] += other.us[i];
            }
        }

        std::string str()const{
            char tmp[256];
            std::string res;
            for(unsigned int i=0; i < ws.size(); ++i){
                snprintf(tmp,256,"%f ",us[i]);
                res += std::string(tmp);
            }
            return res;
        }
};

#endif
