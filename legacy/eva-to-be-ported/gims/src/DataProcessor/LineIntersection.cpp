#include "LineIntersection.hpp"

//helper functions for the linestring intersection matrix construction
void DE9IM_mls_ls(DE9IM *resultset, GIMS_MultiLineString *query, GIMS_LineString *other)
{
    BentleySolver bs;

    GIMS_MultiLineString *other_mls = new GIMS_MultiLineString(1);
    other_mls->append(other);

    list<GIMS_Geometry *> intersections = bs.solve(query, other_mls);
    delete other_mls;

    matrix_t::iterator matrix_index = resultset->getMatrixIndex(other->id);
    if (intersections.size() == 0)
    {
        //then they're disjoint...
        resultset->setIE(matrix_index, 1);
        resultset->setBE(matrix_index, 0);
        resultset->setEI(matrix_index, 1);
        resultset->setEB(matrix_index, 0);
        return;
    }

    GIMS_LineString *other_original = (GIMS_LineString *)(*(idIndex.find(other)));
    GIMS_Point *otherBorder[2] = { other_original->list[0], other_original->list[other_original->size - 1] };

    GIMS_Geometry *query_original = (GIMS_LineString *)(*(idIndex.find(query)));
    GIMS_MultiPoint queryBorder;

    if (query_original->type == LINESTRING)
    {
        GIMS_LineString *aux_ls = (GIMS_LineString *)query_original;
        queryBorder.append(aux_ls->list[0]);
        queryBorder.append(aux_ls->list[aux_ls->size - 1]);
    }
    else if (query_original->type == MULTILINESTRING)
    {
        GIMS_MultiLineString *aux_mls = (GIMS_MultiLineString *)query_original;
        for (int i = 0; i < aux_mls->size; i++)
        {
            queryBorder.append(aux_mls->list[i]->list[0]);
            queryBorder.append(aux_mls->list[i]->list[aux_mls->list[i]->size - 1]);
        }
    }

    int intersectedBorders = 0;

    list<GIMS_LineSegment *> linesegments;

    int shift_inc = queryBorder.size;

    for (list<GIMS_Geometry *>::iterator it = intersections.begin(); it != intersections.end(); it++)
    {
        if ((*it)->type == LINESEGMENT)
        {
            linesegments.push_back((GIMS_LineSegment *)(*it));

            for (int i = 0; i < queryBorder.size; i++)
            {
                if (((GIMS_LineSegment *)(*it))->coversPoint(queryBorder.list[i]))
                    intersectedBorders |= 1 << i;
            }

            for (int i = 0; i < 2; i++)
            {
                if (((GIMS_LineSegment *)(*it))->coversPoint(otherBorder[i]))
                    intersectedBorders |= 1 << (shift_inc + i);
            }

            resultset->setIntersect(matrix_index, 1);
            resultset->setII(matrix_index, 1);
        }
        else
        {
            resultset->setIntersect(matrix_index, 0);

            bool isBorder = false;
            for (int i = 0; i < queryBorder.size; i++)
            {
                if (((GIMS_Point *)(*it))->equals(queryBorder.list[i]))
                {
                    intersectedBorders |= 1 << i;
                    isBorder = true;
                }
            }
            for (int i = 0; i < 2; i++)
            {
                if (((GIMS_Point *)(*it))->equals(otherBorder[i]))
                {
                    intersectedBorders |= 1 << (shift_inc + i);
                    isBorder = true;
                }
            }
            if (!isBorder)
                resultset->setII(matrix_index, 0);
        }
    }

    /*Here we check intersections of Borders with exteriors*/
    if ((intersectedBorders & (1 << shift_inc)) == 0 || (intersectedBorders & (1 << (shift_inc + 1))) == 0)
    {
        resultset->setEB(matrix_index, 0);
    }

    for (int i = 0; i < queryBorder.size; i++)
    {
        if ((intersectedBorders & (1 << i)) == 0)
        {
            resultset->setBE(matrix_index, 0);
            break;
        }
    }

    /*Here we check intersections of exteriors with interiors*/
    bool queryContained = false, otherContained = false;

    if (linesegments.size() >= (unsigned)(query->size))
    {
        //check if linesegments cover the query geometry entirely
        queryContained = query->isCoveredBy(linesegments);
    }

    if (linesegments.size() >= (unsigned)(other->size))
    {
        //check if linesegments cover the other geometry entirely
        otherContained = other->isCoveredBy(linesegments, false);
    }

    if (!queryContained && !otherContained)
    {
        resultset->setEI(matrix_index, 1);
        resultset->setIE(matrix_index, 1);
    }
    else if (!queryContained && otherContained)
    {
        resultset->setIE(matrix_index, 1);
    }
    else if (queryContained && !otherContained)
    {
        resultset->setEI(matrix_index, 1);
    }

    for (list<GIMS_Geometry *>::iterator it = intersections.begin(); it != intersections.end(); it++)
        (*it)->deepDelete();
}

void DE9IM_mls_mls(DE9IM *resultset, GIMS_MultiLineString *query, GIMS_MultiLineString *other)
{
    BentleySolver bs;
    list<GIMS_Geometry *> intersections = bs.solve(query, other);

    matrix_t::iterator matrix_index = resultset->getMatrixIndex(other->id);
    if (intersections.size() == 0)
    {
        //then they're disjoint...
        resultset->setIE(matrix_index, 1);
        resultset->setBE(matrix_index, 0);
        resultset->setEI(matrix_index, 1);
        resultset->setEB(matrix_index, 0);
        return;
    }

    //retrieve other's border
    GIMS_Geometry *original = (*(idIndex.find(other)));
    GIMS_MultiPoint otherBorder;
    if (original->type == LINESTRING)
    {
        GIMS_LineString *orig_ls = (GIMS_LineString *)original;
        otherBorder.append(orig_ls->list[0]);
        otherBorder.append(orig_ls->list[orig_ls->size - 1]);
    }
    else if (original->type == MULTILINESTRING)
    {
        GIMS_MultiLineString *orig_mls = (GIMS_MultiLineString *)original;
        for (int i = 0; i < orig_mls->size; i++)
        {
            otherBorder.append(orig_mls->list[i]->list[0]);
            otherBorder.append(orig_mls->list[i]->list[orig_mls->list[i]->size - 1]);
        }
    }

    //retrieve query's border
    original = (*(idIndex.find(query)));
    GIMS_MultiPoint queryBorder;
    if (original->type == LINESTRING)
    {
        GIMS_LineString *orig_ls = (GIMS_LineString *)original;
        queryBorder.append(orig_ls->list[0]);
        queryBorder.append(orig_ls->list[orig_ls->size - 1]);
    }
    else if (original->type == MULTILINESTRING)
    {
        GIMS_MultiLineString *orig_mls = (GIMS_MultiLineString *)original;
        for (int i = 0; i < orig_mls->size; i++)
        {
            queryBorder.append(orig_mls->list[i]->list[0]);
            queryBorder.append(orig_mls->list[i]->list[orig_mls->list[i]->size - 1]);
        }
    }

    int intersectedBorders = 0;
    list<GIMS_LineSegment *> linesegments;

    int shift_inc = queryBorder.size;

    for (list<GIMS_Geometry *>::iterator it = intersections.begin(); it != intersections.end(); it++)
    {
        if ((*it)->type == LINESEGMENT)
        {
            linesegments.push_back((GIMS_LineSegment *)(*it));

            for (int i = 0; i < queryBorder.size; i++)
            {
                if (((GIMS_LineSegment *)(*it))->coversPoint(queryBorder.list[i]))
                    intersectedBorders |= 1 << i;
            }
            for (int i = 0; i < otherBorder.size; i++)
            {
                if (((GIMS_LineSegment *)(*it))->coversPoint(otherBorder.list[i]))
                    intersectedBorders |= 1 << (shift_inc + i);
            }

            resultset->setIntersect(matrix_index, 1);
            resultset->setII(matrix_index, 1);
        }
        else
        {
            resultset->setIntersect(matrix_index, 0);

            bool isBorder = false;
            for (int i = 0; i < queryBorder.size; i++)
            {
                if (((GIMS_Point *)(*it))->equals(queryBorder.list[i]))
                {
                    intersectedBorders |= 1 << i;
                    isBorder = true;
                }
            }
            for (int i = 0; i < otherBorder.size; i++)
            {
                if (((GIMS_Point *)(*it))->equals(otherBorder.list[i]))
                {
                    intersectedBorders |= 1 << (shift_inc + i);
                    isBorder = true;
                }
            }

            if (!isBorder)
                resultset->setII(matrix_index, 0);
        }
    }

    /*Here we check intersections of Borders with exteriors*/
    for (int i = 0; i < otherBorder.size; i++)
    {
        if ((intersectedBorders & (1 << (shift_inc + i))) == 0)
        {
            resultset->setEB(matrix_index, 0);
            break;
        }
    }

    for (int i = 0; i < queryBorder.size; i++)
    {
        if ((intersectedBorders & (1 << i)) == 0)
        {
            resultset->setBE(matrix_index, 0);
            break;
        }
    }

    /*Here we check intersections of exteriors with interiors*/
    bool queryContained = false, otherContained = false;

    if (linesegments.size() >= (unsigned)(query->size))
    {
        //check if linesegments cover the query geometry entirely
        queryContained = query->isCoveredBy(linesegments);
    }

    int total = 0;
    for (int i = 0; i < other->size; i++)
        total += other->list[i]->size;

    if (linesegments.size() >= (unsigned)(total))
    {
        //check if linesegments cover the other geometry entirely
        otherContained = other->isCoveredBy(linesegments, false);
    }

    if (!queryContained && !otherContained)
    {
        resultset->setEI(matrix_index, 1);
        resultset->setIE(matrix_index, 1);
    }
    else if (!queryContained && otherContained)
    {
        resultset->setIE(matrix_index, 1);
    }
    else if (queryContained && !otherContained)
    {
        resultset->setEI(matrix_index, 1);
    }

    for (list<GIMS_Geometry *>::iterator it = intersections.begin(); it != intersections.end(); it++)
        (*it)->deepDelete();
}
