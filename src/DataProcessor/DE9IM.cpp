#include "DE9IM.hpp"

DE9IM::DE9IM(GIMS_Geometry *query){
    this->query = query;
    matrix = map<long long, unsigned int>();
}

DE9IM::~DE9IM(){}

void DE9IM::setIntersect(long long id, unsigned int dim){
    dim += 1;
    map<long long, unsigned int>::iterator it;
    if( (it = matrix.find(id)) != matrix.end() ){
        it->second |= MAX( it->second & INTERSECT, dim << 0);
    }else
        matrix[id] = dim << 0;
}

void DE9IM::setII(long long id, unsigned int dim){
    dim += 1;
    map<long long, unsigned int>::iterator it;
    if( (it = matrix.find(id)) != matrix.end() )
        it->second |= MAX( it->second & II, dim << 2 );
    else
        matrix[id] = dim << 2;
}

void DE9IM::setEI(long long id, unsigned int dim){
    dim += 1;
    map<long long, unsigned int>::iterator it;
    if( (it = matrix.find(id)) != matrix.end() )
        it->second |= MAX( it->second & EI, dim << 4 );
    else
        matrix[id] = dim << 4;
}

void DE9IM::setIE(long long id, unsigned int dim){
    dim += 1;
    map<long long, unsigned int>::iterator it;
    if( (it = matrix.find(id)) != matrix.end() )
        it->second |= MAX( it->second & IE, dim << 6 );
    else
        matrix[id] = dim << 6;
}

void DE9IM::setEB(long long id, unsigned int dim){
    dim += 1;
    map<long long, unsigned int>::iterator it;
    if( (it = matrix.find(id)) != matrix.end() )
        it->second |= MAX( it->second & EB, dim << 8 );
    else
        matrix[id] = dim << 8;
}

void DE9IM::setBE(long long id, unsigned int dim){
    dim += 1;
    map<long long, unsigned int>::iterator it;
    if( (it = matrix.find(id)) != matrix.end() )
        it->second |= MAX( it->second & BE, dim << 10 );
    else
        matrix[id] = dim << 10;
}

list<long> DE9IM::equals(){
    /*II && !IE && !BE && !EI && !EB*/
    list<long> resultset = list<long>();

    for(map<long long, unsigned int>::iterator it = matrix.begin(); it != matrix.end(); it++){
        unsigned int mask = it->second;
        if(   mask & II &&
            !(mask & IE)&&
            !(mask & BE)&&
            !(mask & EI)&&
            !(mask & EB)
        )
            resultset.push_back( it->first );
    }

    return resultset;
}

/*TODO: in order to implement disjoint geometry reporting, we 
        need access here to all geometries under consideration*/
list<long> DE9IM::disjoint(){
    /*!intersect*/
    return list<long>();
}

list<long> DE9IM::meets(){
    /*!II && intersect*/
    list<long> resultset = list<long>();

    for(map<long long, unsigned int>::iterator it = matrix.begin(); it != matrix.end(); it++){
        unsigned int mask = it->second;
        if(   mask & INTERSECT &&
            !(mask & II)
        )
            resultset.push_back( it->first );
    }

    return resultset;
}

list<long> DE9IM::contains(){
    /*II && !EI && !EB*/
    list<long> resultset = list<long>();

    for(map<long long, unsigned int>::iterator it = matrix.begin(); it != matrix.end(); it++){
        unsigned int mask = it->second;
        if(   mask & II &&
            !(mask & EI)&&
            !(mask & EB)
        )
            resultset.push_back( it->first );
    }

    return resultset;
}

list<long> DE9IM::covers(){
    /*intersect && !EI && !EB*/
    list<long> resultset = list<long>();

    for(map<long long, unsigned int>::iterator it = matrix.begin(); it != matrix.end(); it++){
        unsigned int mask = it->second;
        if(   mask & INTERSECT &&
            !(mask & EI)&&
            !(mask & EB)
        )
            resultset.push_back( it->first );
    }

    return resultset;
}

list<long> DE9IM::intersects(){
    /*II || IB || BI || BB*/
    list<long> resultset = list<long>();

    for(map<long long, unsigned int>::iterator it = matrix.begin(); it != matrix.end(); it++){
        unsigned int mask = it->second;
        if( mask & INTERSECT )
            resultset.push_back( it->first );
    }

    return resultset;
}

list<long> DE9IM::within(){
    /*II && !IE && !BE*/
    list<long> resultset = list<long>();

    for(map<long long, unsigned int>::iterator it = matrix.begin(); it != matrix.end(); it++){
        unsigned int mask = it->second;
        if(   mask & II &&
            !(mask & IE)&&
            !(mask & BE)
        )
            resultset.push_back( it->first );
    }

    return resultset;
}

list<long> DE9IM::coveredBy(){
    /*intersect && !IE && !BE*/
    list<long> resultset = list<long>();

    for(map<long long, unsigned int>::iterator it = matrix.begin(); it != matrix.end(); it++){
        unsigned int mask = it->second;
        if(   mask & INTERSECT &&
            !(mask & IE)&&
            !(mask & BE)
        )
            resultset.push_back( it->first );
    }

    return resultset;
}


/*TODO: be able to get the original geometry from the id in order to assess the dimension

list<long> DE9IM::crosses(){
    int dim_a = dim(query), dim_b;

    list<long> resultset = list<long>();
    for(AVLTree::iterator it = II->begin(); it != II->end(); it++){
        dim_b = dim(*it);

        if(dim_a == 0 || dim_b == 0){
            //II
            resultset.push_back((*it)->id);
        
        }else if(dim_a < dim_b &&
                IE->find((*it)->id) != NULL){
            //II && IE
            resultset.push_back((*it)->id);

        }else if(dim_b > dim_a && 
                EI->find((*it)->id) != NULL){
            //II && EI
            resultset.push_back((*it)->id);
        }
    }
    return resultset;
}
*/

TODO(be able to get the original geometry from the id in order to assess the dimension)
/*
list<long> DE9IM::overlaps(){

    int dim_a = dim(query), dim_b;

    list<long> resultset = list<long>();
    for(map<long long, unsigned int>::iterator it = matrix.begin(); it != matrix.end(); it++){
        dim_b = ???
        unsigned int mask = it->second;

        if( dim_a == dim_b ){
            if(dim_a == 0 || dim_a == 2){
                //II && IE && EI
                if( mask & II &&
                    mask & IE &&
                    mask & EI )
                    resultset.push_back((*it)->id);
            }else{
                //II == 1 && IE && EI
                if( DIM_II(mask) == 1 &&
                    mask & IE         &&
                    mask & EI          )
                    resultset.push_back((*it)->id);
            }
        }
    }
    return resultset;
}
*/
