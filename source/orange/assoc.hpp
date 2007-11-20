/*
    This file is part of Orange.

    Orange is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Orange is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Orange; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Authors: Matjaz Jursic, Janez Demsar, Blaz Zupan, 1996--2002
    Contact: janez.demsar@fri.uni-lj.si

*/


#ifndef _ASSOC_HPP
#define _ASSOC_HPP

#include <iostream>

#include "stladdon.hpp"

#include "vars.hpp"
#include "domain.hpp"
#include "examples.hpp"
#include "classify.hpp"
#include "learn.hpp"

WRAPPER(Example)

class ORANGE_API TAssociationRule : public TOrange {
public:
  __REGISTER_CLASS

  PExample left; //PR left side of the rule
  PExample right; //PR right side of the rule
  float support; //P support for the rule
  float confidence; //P confidence of the rule
  float coverage; //P rule's coverage
  float strength; //P rule's strength
  float lift; //P rule's lift
  float leverage; //P rule's leverage
  float nAppliesLeft; //P number of examples covered by the rule's left side 
  float nAppliesRight; //P number of examples covered by the rule's right side 
  float nAppliesBoth; //P number of examples covered by the rule
  float nExamples; //P number of learning examples
  int nLeft; //PR number of items on the rule's left side
  int nRight; //PR number of items on the rule's right side

  TAssociationRule(PExample, PExample);

  TAssociationRule(PExample al, PExample ar,
                   const float &napLeft, const float &napRight, const float &napBoth, const float &nExamples,
                   int anleft=-1, int anright=-1);

  virtual bool operator ==(const TAssociationRule &other) const
  { return (left->operator==(other.left.getReference())) && (right->operator==(other.right.getReference())); }

  static bool applies(const TExample &ex, const PExample &side);

  bool appliesLeft(const TExample &ex) const
  { return applies(ex, left); }

  bool appliesRight(const TExample &ex) const
  { return applies(ex, right); }

  bool appliesBoth(const TExample &ex) const
  { return applies(ex, left) && applies(ex, right); }

  static int countItems(PExample ex);
};

WRAPPER(AssociationRule)

#ifdef _MSC_VER
  ORANGE_EXTERN template class ORANGE_API TOrangeVector<PAssociationRule>;
#endif

#define TAssociationRules TOrangeVector<PAssociationRule>
VWRAPPER(AssociationRules)


class TItemSetNode;
class TRuleTreeNode;

class ORANGE_API TAssociationRulesInducer : public TOrange {
public:
  __REGISTER_CLASS

  int maxItemSets; //P maximal number of itemsets (increase if you want)

  float confidence; //P required confidence
  float support; //P required support
  bool classificationRules; //P if true, rules will have the class and only the class attribute on the right-hand side

public:

  TAssociationRulesInducer(float asupp=0.1, float aconf=0.5);
  PAssociationRules operator()(PExampleGenerator, const int &weightID = 0);

  void buildTrees(PExampleGenerator, const int &weightID, TItemSetNode *&, int &depth, int &nOfExamples, TDiscDistribution &);
  int  buildTree1(PExampleGenerator, const int &weightID, TItemSetNode *&, float &suppN, int &nOfExamples, TDiscDistribution &);
  int  buildNext1(TItemSetNode *, int k, const float suppN);
  int  makePairs (TItemSetNode *, const float suppN);

  PAssociationRules generateClassificationRules(PDomain, TItemSetNode *tree, const int nOfExamples, const TDiscDistribution &);
  void generateClassificationRules1(PDomain, TItemSetNode *root, TItemSetNode *node, TExample &left, const int nLeft, const float nAppliesLeft, PAssociationRules, const int nOfExamples, const TDiscDistribution &);

  PAssociationRules generateRules(PDomain, TItemSetNode *, const int depth, const int nOfExamples);
  void generateRules1(TExample &, TItemSetNode *root, TItemSetNode *node, int k, int oldk, PAssociationRules, const int nOfExamples);
  void find1Rules(TExample &, TItemSetNode *, const float &support, int oldk, PAssociationRules, const int nOfExamples);
  TRuleTreeNode *buildTree1FromExample(TExample &, TItemSetNode *);
  int generateNext1(TRuleTreeNode *ruleTree, TRuleTreeNode *node, TItemSetNode *itemsetsTree, TExample &right, int k, TExample &whole, const float &support, PAssociationRules, const int nOfExamples);
  int generatePairs(TRuleTreeNode *ruleTree, TRuleTreeNode *node, TItemSetNode *itemsetsTree, TExample &right, TExample &whole, const float &support, PAssociationRules, const int nOfExamples);
};

WRAPPER(AssociationRulesInducer)




class TSparseExample{
public:
	float weight;			// weight of thi example
	long *itemset;		// vector storing just items that have some value in original example
	int	length;

	TSparseExample(TExample *example, int weightID);
};


class TSparseExamples{
public:
	float fullWeight;					// weight of all examples
	vector<TSparseExample*> transaction;	// vector storing all sparse examples
	PDomain domain;						// domain of original example or exampleGenerator
	vector<long> intDomain;				// domain mapped longint values

	TSparseExamples(PExampleGenerator examples, int weightID);
};


class TSparseItemsetNode;
typedef map<long, TSparseItemsetNode *> TSparseISubNodes;

class TSparseItemsetNode{							//item node used in TSparseItemsetTree
public:
	float weiSupp;							//support of itemset consisting node and all of its parents
	long value;								//value of this node
	TSparseItemsetNode *parent;					//pointer to parent node
	TSparseISubNodes subNode;				//children items
	
	TSparseItemsetNode(long avalue = -1);			//constructor

    TSparseItemsetNode *operator[] (long avalue);	//directly gets subnode

	TSparseItemsetNode* addNode(long avalue);		//adds new subnode
	bool hasNode(long avalue);				//returns true if has subnode with given value
};


class TSparseItemsetTree : TOrange {							//item node used in TSparseItemsetTree
public:
	TSparseItemsetTree(TSparseExamples examples);			//constructor

	int buildLevelOne(vector<long> intDomain);
	long extendNextLevel(int maxDepth, long maxCount);
	bool allowExtend(long itemset[], int iLength);
	long countLeafNodes();
	void considerItemset(long itemset[], int iLength, float weight, int aimLength);
	void considerExamples(TSparseExamples *examples, int aimLength);
	void delLeafSmall(float minSupport);
	PAssociationRules genRules(int maxDepth, float minConf, float nOfExamples);
	long getItemsetRules(long itemset[], int iLength, float minConf, 
						 float nAppliesBoth, float nOfExamples, PAssociationRules rules);
	PDomain domain;

//private:
	TSparseItemsetNode *root;
};


class ORANGE_API TAssociationRulesSparseInducer : public TOrange {
public:
  __REGISTER_CLASS

  int maxItemSets; //P maximal number of itemsets (increase if you want)

  float confidence; //P required confidence
  float support; //P required support

  TAssociationRulesSparseInducer(float asupp=0.1, float aconf=0, int awei=0);
  PAssociationRules operator()(PExampleGenerator, const int &weightID);

private:
  float nOfExamples;
};

WRAPPER(AssociationRulesSparseInducer)


WRAPPER(SparseItemsetTree)

class ORANGE_API TItemsetsSparseInducer : public TOrange {
public:
  __REGISTER_CLASS

  int maxItemSets; //P maximal number of itemsets (increase if you want)
  float support; //P required support

  TItemsetsSparseInducer(float asupp=0.1, int awei=0);
  PSparseItemsetTree operator()(PExampleGenerator, const int &weightID);

private:
  float nOfExamples;
};


class ORANGE_API TAssociationLearner : public TLearner {
public:
  __REGISTER_CLASS

  float confidence; //P required confidence
  float support; //P required support
  int voteWeight; //P vote weight (s=support, c=confidence, p=product)
  int maxItemSets; //P maximal number of itemsets (increase if you want)
 
  TAssociationLearner();
  virtual PClassifier operator()(PExampleGenerator gen, const int & = 0);
};


class ORANGE_API TAssociationClassifier : public TClassifierFD {
public:
  __REGISTER_CLASS

  PAssociationRules rules; //P association rules
  int voteWeight; //P vote weight (s=support, c=confidence, p=product)
 
  TAssociationClassifier(PDomain dom=PDomain(), PAssociationRules arules=PAssociationRules(), char avote='s');
  virtual PDistribution classDistribution(const TExample &);
};

#endif
