#!/usr/bin/env python
# coding: utf-8

# In[2]:


pip install lightgbm


# In[3]:


pip install xgboost


# # Introduction 
# 

# # Objective 
# 
# ###### Predict if a driver will file an insurance claim next year

# # Data
# 
#  - Source of the data is kaggle, Porto Seguro Driver's Prediction Challenge. 
#  - Features that belong to similar groupings are tagged as such in the feature names (e.g., ind, reg, car, calc).
#  - Feature names include the postfix bin to indicate binary features and cat to indicate categorical features.
#  - Features without these designations are either continuous or ordinal.
#  - Values of -1 indicate that the feature was missing from the observation.
#  - The target columns signifies whether or not a claim was filed for that policy holder.
# 

# 
# 
# # Import packages 

# In[4]:


# libraries
import pandas as pd 
import numpy as np
import seaborn as sns
import scipy as sp 
import matplotlib.pyplot as plt

from sklearn.preprocessing import Imputer
from sklearn.pipeline import Pipeline
from sklearn.pipeline import make_pipeline
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import GridSearchCV
from sklearn.model_selection import RandomizedSearchCV


from sklearn.model_selection import train_test_split
from sklearn.linear_model import LogisticRegression
from sklearn.ensemble import RandomForestClassifier
from xgboost import XGBClassifier
from sklearn.model_selection import cross_val_score
from lightgbm import LGBMClassifier
import lightgbm as lgb

from sklearn.pipeline import Pipeline

#import xgboost as xgb
#from sklearn.model_selection import KFold
#from sklearn.model_selection import train_test_split


# # Load Date

# In[5]:


train  = pd.read_csv('C:/Users/royag/Desktop/Resume and Cover Letter/Interview projects/train.csv')
test = pd.read_csv('C:/Users/royag/Desktop/Resume and Cover Letter/Interview projects/test.csv')


# In[6]:


#print (train.shape)
#print (test.shape)

print ('train set: # of rows = {} and # of columns = {}' .format (train.shape[0], train.shape[1]))
print ('test set:  # of rows = {} and # of columns = {}' .format (test.shape[0], test.shape[1]))


# In[7]:


#train.head()
train.columns


# In[5]:


#train.tail()


# ## Duplicate rows
# 
# Look at the number of rows and columns in the train data and the test data.
# Check if we have the same number of variables in the test data.
# Check if there are duplicate rows in the training data.

# In[8]:


print (train.shape)
print (test.shape)

train.drop_duplicates()

print (train.shape)


# - Target variable is missing in test data set
# - There is no duplicate rows 

# ## Data type
# 
# Invesigate how many variables of each type we have.

# In[9]:


train.info()


# In[10]:


#make a copy of data sets
train_set=train.copy()
test_set=test.copy()


# ### Variable information in a dataframe
# 
# - **role**: input, ID, target 
# - **level**: Categorical, Real, Integer, Binary 
# - **drop**: True or False
# - **category**: calculated, individual, registration, car
# - **dtype**: int, float, str

# In[11]:


data=[]
for var in train_set.columns:
    # define the role of the features
    if var=='target':
        role='target'
    elif var=='id':
        role='id'
    else:
        role='input'
        
    #define the level of the features
    if 'bin' in var or var=='target':
        level='binary'
        #print (level)
    elif 'cat' in var or var=='id':
        level='categorical'
    elif train_set[var].dtype==float:
        level = 'real'
    else:
        level='integer'
        
    # Defining the category of the features
    category='00'
    if 'ind' in var:
        category = 'individual'
    elif 'reg' in var:
        category = 'registration'
    elif 'car' in var:
        category = 'car'
    else:
        category = 'calculated'

    # define new boolean var to decide about keeping or dropping the features
    drop=False
    if var=='id':
        drop=True

    # define type of the features
    dtype = train_set[var].dtype

    my_dic= {'varname':var, 'role': role, 'level':level, 'drop':drop, 'category':category, 'dtype': dtype}

    data.append (my_dic)
        
table_1= pd.DataFrame(data , columns=['varname', 'role' , 'level', 'drop', 'category', 'dtype'])

table_1.set_index('varname', inplace=True)


# In[10]:


table_1


# In[12]:


# extract categorical data

table_1[(table_1.level == 'categorical')].index


# In[13]:


# number of variables per role and level

pd.DataFrame({'count': table_1.groupby(['level', 'dtype'])['level'].size()}).reset_index()


# # Exploring the data
# 
# 
# ## - Target value 

# In[14]:



plt.figure()
fig, ax = plt.subplots(figsize=(6,6))
x = train_set['target'].value_counts().index.values
y = train_set['target'].value_counts().values
# Bar plot
# Order the bars descending on target mean
sns.barplot(ax=ax, x=x, y=y)
plt.ylabel('Frequency', fontsize=12)
plt.xlabel('Target value', fontsize=12)
plt.tick_params(axis='both', which='major', labelsize=12)
for p in ax.patches:
    percentage = '{:.2f}%'.format(100 * p.get_height()/len(train_set))
    ax.annotate(percentage,(p.get_x() + p.get_width()/2 , p.get_y()+ p.get_height()),                    ha = 'center', va = 'center', xytext = (0, 9),textcoords = 'offset points')
# for p in ax.patches:
#        ax.text(p.get_x(), p.get_height(), str(round((p.get_height()/len(train_set))*100, 2))+'%', fontsize=15, color='black',\
#                ha='center' , va='center')

plt.show();


# In[15]:


train_set['target'].value_counts()


# ## - Categorical Varibles 

# In[16]:


cat_vars = [e for e in train_set.columns if e.endswith('cat')]

for e in cat_vars:
    dist_values = train_set[e].value_counts().shape[0]
    print('Variable {} has {} distinct values'.format(e, dist_values))


# In[17]:



for f in cat_vars:
    print (f)
    fig, ax = plt.subplots(figsize=(6,6))
    # Calculate the percentage of target=1 per category value
    cat_percentage = train_set[[f, 'target']].groupby([f],as_index=False).mean()
    cat_percentage.sort_values(by='target', ascending=False, inplace=True)
    # Bar plot
    # Order the bars descending on target mean
    sns.barplot(ax=ax,x=f, y='target', data=cat_percentage, order=cat_percentage[f])
    plt.ylabel('Percent of target with value 1 [%]', fontsize=12)
    plt.xlabel(f , fontsize=12)
    plt.tick_params(axis='both', which='major', labelsize=12)
    plt.show();


# # Data Preprocessing
# 

# In[17]:


#train_set.iloc[:, 2:].columns
# for i in range(train_set.shape[1]-2):
#     plt.boxplot(train_set.iloc[:, i])
#     plt.show()


# ## - check missing values 

# In[18]:



missingValues1 = []
missingPer1=[]

for i in train_set.columns:
    missings1 = train_set[train_set[i] == -1][i].count()
    if missings1 > 0:
        missingValues1.append(i)
        missingPer1.append(missings1/train_set.shape[0])
        missingPercent1 = missings1/train_set.shape[0]
        
        print('Variable {} in train data has {} records ({:.2%}) with missing values'.format(i, missings1, missingPercent1))
        
print('In total, there are {} variables with missing values in train data'.format(len(missingValues1)))


missingValues2 = []
missingPer2=[]


for i in test_set.columns:
    missings2 = test_set[test_set[i] == -1][i].count()
    if missings2 > 0:
        missingValues2.append(i)
        missingPer2.append(missings2/test_set.shape[0])
        missingPercent2 = missings2/test_set.shape[0]
        
        print('Variable {} in test data has {} records ({:.2%}) with missing values'.format(i, missings2, missingPercent2))
        
print('In total, there are {} variables with missing values in test data'.format(len(missingValues2)))





# ### - Visualize the missing values of the features 

# In[19]:


missing_vars1 = pd.DataFrame(sorted(zip(missingPer1, missingValues1)), columns=["Value", "Feature"])
plt.figure(figsize=(16, 5))
sns.barplot(x="Value", y="Feature", data=missing_vars1.sort_values(by="Value", ascending=False))


missing_vars2 = pd.DataFrame(sorted(zip(missingPer2, missingValues2)), columns=["Value", "Feature"])
plt.figure(figsize=(16, 5))
sns.barplot(x="Value", y="Feature", data=missing_vars2.sort_values(by="Value", ascending=False))


# ### - Drop variables with too many missing values

# In[20]:



vars_drop = ['ps_car_03_cat', 'ps_car_05_cat']
#vars_drop = ['ps_reg_03', 'ps_car_14']
train_set.drop(vars_drop, inplace=True, axis=1)
test_set.drop(vars_drop, inplace=True, axis=1)

#table_1.loc[(vars_drop),'drop'] = True  # Updating table_1


# ### - Replace -1 values with NaN

# In[21]:



train_set = train_set.replace(-1, np.nan)
test_set = test_set.replace(-1, np.nan)


# ### - Imputing the missing values

# In[22]:


cat_cols = [c for c in train_set.columns if 'cat' in c]
bin_cols = [c for c in train_set.columns if 'bin' in c]
con_cols = [c for c in train_set.columns if c not in (bin_cols + cat_cols) ]   #real and integer
del con_cols[:2]


print (con_cols)

# Imputing with the mean or mode 
mean_imp = Imputer(missing_values=np.nan , strategy='mean', axis=0)  
mode_imp = Imputer(missing_values=np.nan , strategy='most_frequent', axis=0)

for c in cat_cols:
    train_set[c] = mode_imp.fit_transform(train_set[[c]]).ravel()
    test_set[c] = mode_imp.transform(test_set[[c]]).ravel()
    
for c in bin_cols:
    train_set[c] = mean_imp.fit_transform(train_set[[c]]).ravel()
    test_set[c] = mean_imp.transform(test_set[[c]]).ravel()
    
for c in con_cols:
    train_set[c] = mean_imp.fit_transform(train_set[[c]]).ravel()
    test_set[c] = mean_imp.transform(test_set[[c]]).ravel()  
    
"""
for c in cat_cols:
    train_set[c].fillna(value=train_set[c].mode()[0], inplace=True)
    test_set[c].fillna(value=test_set[c].mode()[0], inplace=True)
    
    
for c in bin_cols:
    train_set[c].fillna(value=train_set[c].mode()[0], inplace=True)
    test_set[c].fillna(value=test_set[c].mode()[0], inplace=True)
    
for c in con_cols:
    train_set[c].fillna(value=train_df[c].mean(), inplace=True)
    test_set[c].fillna(value=test_df[c].mean(), inplace=True)

"""


# ## - Corrolation between the variables 

# In[23]:



correlations = train_set.corr()

# Create color map ranging between two colors
cmap = sns.diverging_palette(200, 10, as_cmap=True)

fig, ax = plt.subplots(figsize=(10,10))
sns.heatmap(correlations, cmap=cmap, vmax=1, center=0, fmt='.2f',
                square=True, linewidths=.25, annot=False, cbar_kws={"shrink": .5})
plt.show();
    


# ### - Find highly correlated Vars 

# In[24]:


for i in correlations.columns:
    for j in correlations.columns:
        if (correlations[i][j])>0.5 and j>i:
            print ('{} and {} ({:.2%})'.format (i , j , correlations[i][j]))


# In[27]:


#vars_drop = ['ps_ind_14', 'ps_car_13', 'ps_car_04_cat']

# train_set.drop(vars_drop, inplace=True, axis=1)
# test_set.drop(vars_drop, inplace=True, axis=1)


# ## - Dummify categorical variables 

# In[25]:


cat_vars  = [e for e in train_set.columns if e.endswith('cat')]

def encode_cat_vars(df, cat_vars):
    for c in cat_vars:
        temp = pd.get_dummies(pd.Series(df[c]), prefix=c)        
        df = pd.concat([df, temp],axis=1)
        df = df.drop([c],axis=1)
    return df

train_set = encode_cat_vars(train_set, cat_vars)
test_set = encode_cat_vars(test_set, cat_vars)


# In[26]:


print(train_set.shape)
print(test_set.shape)


# In[27]:


train_target = train_set['target']
train_target_value = train_set['target'].values
train_y= np.array(train['target'])
train_id= train['id']
train_id_value= train['id'].values
test_id=test['id']
test_id_value=test['id'].values


# ### - Drop "id" and "target" columns from both train and test set

# In[28]:


train_set = train_set.drop(['target','id'], axis = 1)
test_set = test_set.drop(['id'], axis = 1)


# ## - Feature Selection

# In[29]:



model = LGBMClassifier(n_estimators=2000, learning_rate=0.1, max_depth=-1, min_data_in_leaf = 1, min_sum_hessian_in_leaf = 1.0)
#model= RandomForestClassifier(n_estimators=1000, random_state=0, n_jobs=-1)
X = train_set
y = train_target_value
#y = train_target

model.fit(X, y)

features_imp = pd.DataFrame(sorted(zip(model.feature_importances_, X.columns)), columns=["Value", "Feature"])

print (features_imp)

plt.figure(figsize=(16, 50))
sns.barplot(x="Value", y="Feature", data=features_imp.sort_values(by="Value", ascending=False))


# ### - Drop the features that are not significant

# In[30]:


sorted_columns = sorted(zip(model.feature_importances_, X.columns))
# print (sorted_columns_by_fi)
col_to_drop = [x[1] for x in sorted_columns if x[0] < 100]
#col_to_drop = [x[1] for x in sorted_columns if x[0] < 0.0025]
#print (col_to_drop)
train_set_imp = train_set.drop(col_to_drop, axis=1)
test_set_imp = test_set.drop(col_to_drop, axis=1)


# ### - Drop calculated features 

# In[31]:


col_drop = train_set.columns[train_set.columns.str.startswith('ps_calc_')]
train_WOCalc = train_set_imp.drop(col_drop, axis=1)  
test_WOCalc = test_set_imp.drop(col_drop, axis=1)  


# In[32]:


print(train_set_imp.shape)
print(train_WOCalc.shape)
print(test_set_imp.shape)
print(test_WOCalc.shape)


# # Scaling and preparing models 

# ### XGBoost 

# In[33]:


pipe_steps = [('scaler', StandardScaler()), ('xgboost',XGBClassifier())] 

params = {
        'xgboost__min_child_weight': [5, 10],
        'xgboost__gamma': [2, 5],
        'xgboost__subsample': [ 0.8, 1.0],
        'xgboost__colsample_bytree': [0.8, 1.0],
        'xgboost__max_depth': [4, 5]
        }

pipeline = Pipeline(pipe_steps)

#X= train_set_imp
X = train_WOCalc
y = train_target_value



# In[34]:


cv_xgb  = GridSearchCV(pipeline, params, cv=10, scoring='roc_auc')
#cv_xgb = RandomizedSearchCV(pipeline, params, cv=10, scoring='roc_auc')


# In[35]:


print (cv_xgb)


# In[ ]:


cv_xgb.fit(X, y)

print(cv_xgb.best_params_)
print(cv_xgb.best_score_)


# In[40]:


y_test = cv_xgb.predict_proba(test_WOCalc)
results_df = pd.DataFrame(data={'id':test['id'], 'target':y_test[:,1]})
results_df.to_csv('submission-grid-search-xgb-cv10-porto-01.csv', index=False)


# ### LGBM 

# In[42]:



pipe_steps = [('scaler', StandardScaler()), ('LGBM',LGBMClassifier())] 


params = {
        'LGBM__n_estimators':[50, 100, 500, 1000]
        }

pipeline = Pipeline(pipe_steps)

#X= train_set_imp
X = train_WOCalc
y = train_target_value


# In[43]:


cv_lgbm = GridSearchCV(pipeline, params, cv=10, scoring='roc_auc')
#cv = RandomizedSearchCV(pipeline, params, cv=5)


# In[44]:


cv_lgbm.fit(X, y)

print(cv_lgbm.best_params_)
print(cv_lgbm.best_score_)


# In[45]:


y_test = cv_lgbm.predict_proba(test_WOCalc)

results_df = pd.DataFrame(data={'id':test['id'], 'target':y_test[:,1]})
results_df.to_csv('submission-grid-search-LGBM-cv10-porto-01.csv', index=False)


# ### Random forest 

# In[46]:



pipe_steps = [('scaler', StandardScaler()), ('RF', RandomForestClassifier())] 

params = { #'features__text__tfidf__max_df': [0.9, 0.95],
                    #'features__text__tfidf__ngram_range': [(1,1), (1,2)],
                    #'RF__learning_rate': [0.1, 0.2],
                    'RF__n_estimators': [30, 50],
                    'RF__max_depth': [2, 4],
                    'RF__min_samples_leaf': [2, 4]}
    
pipeline = Pipeline(pipe_steps)
    
    
#X= train_set_imp
X = train_WOCalc
y = train_target


# In[47]:


cv_rf = GridSearchCV(pipeline, params, cv=10, scoring='roc_auc')
#cv = RandomizedSearchCV(pipeline, params, cv=5)


# In[48]:


cv_rf.fit(X, y)

print(cv_rf.best_params_)
print(cv_rf.best_score_)


# In[49]:


y_test = cv_rf.predict_proba(test_WOCalc)
results_df = pd.DataFrame(data={'id':test['id'], 'target':y_test[:,1]})
results_df.to_csv('submission-grid-search-RF-CV10-porto-01.csv', index=False)


# ### Logistic regression

# In[50]:



pipe_steps = [('scaler', StandardScaler()), ('lg', LogisticRegression())] 

params = {'lg__solver': ['newton-cg', 'sag', 'lbfgs']
         # ,'lg__multi_class':['ovr', 'multinomial']
         }
    
pipeline = Pipeline(pipe_steps)
    
    
#X= train_set_imp
X = train_WOCalc
y = train_target


# In[51]:


cv_lg = GridSearchCV(pipeline, params, cv=10, scoring='roc_auc')
#cv = RandomizedSearchCV(pipeline, params, cv=5)


# In[52]:


cv_lg.fit(X, y)

print(cv_lg.best_params_)
print(cv_lg.best_score_)


# In[53]:


y_test = cv_lg.predict_proba(test_WOCalc)
results_df = pd.DataFrame(data={'id':test['id'], 'target':y_test[:,1]})
results_df.to_csv('submission-grid-search-lg-cv10-porto-01.csv', index=False)


# In[ ]:





# In[101]:


results_df.head()


# In[102]:


results_df.to_csv('submission-random-grid-search-LGBM-porto-01.csv', index=False)


# In[41]:


import os
os.getcwd()


# In[1]:


cd "C:\\Users\\royag\\Desktop\\Resume and Cover Letter\\Interview projects"


# In[100]:


# Function including all classifiers

model=[('XGBoost', XGBClassifier())]
       #, 
#     ('Randomforest', RandomForestClassifier()),
#     ('LogisticRegression', LogisticRegression()),
#     ('lightgbm',LGBMClassifier())]

p_xgboost= {
        'xgboost__min_child_weight': [5, 10],
        'xgboost__gamma': [2, 5],
        'xgboost__subsample': [ 0.8, 1.0],
        'xgboost__colsample_bytree': [0.8, 1.0],
        'xgboost__max_depth': [4, 5]
        }

p_rf= {#'RF__learning_rate': [0.1, 0.2],
                    'RF__n_estimators': [30, 50],
                    'RF__max_depth': [2, 4],
                    'RF__min_samples_leaf': [2, 4]}
   

p_lg= params = {'lg__solver': ['newton-cg', 'sag', 'lbfgs']
         # ,'lg__multi_class':['ovr', 'multinomial']
         }


p_lgbm= {
        'LGBM__n_estimators':[50, 100, 500, 1000]
        }

params = [p_xgboost]
         # , p_rf, p_lg, p_lgbm]
cv_range=[3,5,10]
    
# scoring = ['accuracy', 'precision_weighted', 'recall_weighted', 'f1_weighted', 'roc_auc']



def Best_predict (X,y): 
    b_score=0
    for i in range(len (model)):
        for c in cv_range: 
            pipe_steps = [('scaler', StandardScaler()), (model[i])] 
            cv = GridSearchCV(pipeline, params[i], cv=c, scoring='roc_auc')
            #cv = RandomizedSearchCV(pipeline, params, cv=c, scoring='roc_auc')
            try: 
                cv.fit(X, y)
                if cv.best_score_ > b_score:
                    b_score = cv.best_score_ 
                    b_params=cv.best_params_
                    b_model=i
                    y_test = cv.predict_proba(test_WOCalc)

            #print ('model {} best parameters are {} and best score iis {}' .format(i, cv.best_params_, cv.best_score_)
            except: 
                print(str(i) + "  pass")
                pass
           
    print ('more accurate model is {}, best parameters are {}, and best score is {}'.format( b_model, b_params, b_score))
    results_df = pd.DataFrame(data={'id':test['id'], 'target':y_test[:,1]})
    results_df.to_csv('submission-random-grid-search-'+ str(b_model) +'-porto-01.csv', index=False)



# In[101]:


Best_predict(train_WOCalc, train_target)


# ## GridsearchCV

# In[66]:


# cv = GridSearchCV(pipeline, params, cv=10, scoring='roc_auc')
# #cv = RandomizedSearchCV(pipeline, params, cv=5)


# ### Fitting on train set

# In[ ]:


# cv.fit(X, y)

# print(cv.best_params_)
# print(cv.best_score_)


# ### Prediction and preparing the output

# In[85]:


# y_test = cv.predict_proba(test_WOCalc)
# results_df = pd.DataFrame(data={'id':test['id'], 'target':y_test[:,1]})
# results_df.to_csv('submission-random-grid-search-LGBM-porto-01.csv', index=False)

