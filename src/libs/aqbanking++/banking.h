/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQ_BANKING_CPP_H
#define AQ_BANKING_CPP_H


#include <aqbanking/banking.h>
#include <list>
#include <string>


namespace AB {

  /**
   * This class simply is a C++ binding for the C module @ref AB_BANKING.
   * It redirects C callbacks used by AB_BANKING to virtual functions in
   * this class. It als transforms some return values inconveniant for
   * C++ into STL objects (such as "list<T>").
   *
   * @author Martin Preuss<martin@aquamaniac.de>
   */
  class Banking {
    friend class Banking_Linker;

  private:
    AB_BANKING *_banking;

  protected:
    /**
     * See @ref AB_Banking_MessageBox
     */
    virtual int messageBox(GWEN_TYPE_UINT32 flags,
                           const char *title,
                           const char *text,
                           const char *b1,
                           const char *b2,
                           const char *b3);

    /**
     * See @ref AB_Banking_InputBox
     */
    virtual int inputBox(GWEN_TYPE_UINT32 flags,
                         const char *title,
                         const char *text,
                         char *buffer,
                         int minLen,
                         int maxLen);

    /**
     * See @ref AB_Banking_ShowBox
     */
    virtual GWEN_TYPE_UINT32 showBox(const char *title,
                                     const char *text);
    /**
     * See @ref AB_Banking_HideBox
     */
    virtual void hideBox(GWEN_TYPE_UINT32 id);

    /**
     * See @ref AB_Banking_ProgressStart
     */
    virtual GWEN_TYPE_UINT32 progressStart(const char *title,
                                           const char *text,
                                           GWEN_TYPE_UINT32 total);

    /**
     * See @ref AB_Banking_ProgressAdvance
     */
    virtual int progressAdvance(GWEN_TYPE_UINT32 id,
                                GWEN_TYPE_UINT32 progress);
    /**
     * See @ref AB_Banking_ProgressLog
     */
    virtual int progressLog(GWEN_TYPE_UINT32 id,
                            AB_BANKING_LOGLEVEL level,
                            const char *text);
    /**
     * See @ref AB_Banking_ProgressEnd
     */
    virtual int progressEnd(GWEN_TYPE_UINT32 id);


  public:
    Banking(const char *appname,
            const char *fname);
    virtual ~Banking();


    /**
     * See @ref AB_Banking_Init
     */
    int init();

    /**
     * See @ref AB_Banking_Fini
     */
    int fini();


    /**
     * This function loads the given backend (if it not already has been) and
     * imports any account that backend might offer. You can use this function
     * to engage a backend which has not yet been used (but it doesn't hurt if you
     * use it on already active backends).
     */
    int importProviderAccounts(const char *backend);


    /**
     * Returns the application name as given to @ref AB_Banking_new.
     */
    const char *getAppName();


    /**
     * Tries to load the wizard for the given backend which is of the given
     * type t.
     * Setup wizards are sorted by backends, since the wizard do things
     * very much dependant on the backend. Ideally they are shipped with the
     * backend.
     * @param ab pointer to the AB_BANKING object
     * @param pn name of the backend
     * @param t wizard type. To allow keeping the API as open as possible you
     * may give a type name here. However, the following names are expected:
     * <ul>
     *  <li><b>kde</b> for a wizard running under KDE</li>
     *  <li><b>gnome</b> for a wizard running under GNOME</li>
     *  <li><b>console</b> for a wizard running in a console</li>
     *  <li><b>curses</b> for a wizard using (n)curses</li>
     * </ul>
     */
    AB_PROVIDER_WIZARD *getWizard(const char *pn, const char *t);

    /**
     * Returns a list of pointers to currently known accounts.
     * Please note that the pointers in this list are still owned by
     * AqBanking, so you MUST NOT free them.
     * However, destroying the list will not free the accounts, so it is
     * safe to do that.
     */
    std::list<AB_ACCOUNT*> getAccounts();

    /**
     * This function does an account lookup based on the given unique id.
     * This id is assigned by AqBanking when an account is created.
     * The pointer returned is still owned by AqBanking, so you MUST NOT free
     * it.
     */
    AB_ACCOUNT *getAccount(GWEN_TYPE_UINT32 uniqueId);

    /**
     * Returns a GWEN_DB_NODE which can be used to store/retrieve data for
     * the currently running application. The group returned MUST NOT be
     * freed.
     * AqBanking is able to separate and store the data for every application.
     */
    GWEN_DB_NODE *getAppData();



    /** @name Plugin Handling
     *
     */
    /*@{*/
    /**
     * Returns a list of provider descriptions.
     * You must free the contents of the list after using it via
     * @ref clearPluginDescrs() before deleting the list itself.
     */
    std::list<GWEN_PLUGIN_DESCRIPTION*> getProviderDescrs();

    /**
     * Returns a list of wizard descriptions for the given backend.
     * You must free the contents of the list after using it via
     * @ref clearPluginDescrs() before deleting the list itself.
     */
    std::list<GWEN_PLUGIN_DESCRIPTION*> getWizardDescrs(const char *pn);

    /**
     * Frees all plugin descriptions whose pointers are stored inside
     * the given list.
     * Please note that this methode renders the list useless, so it should
     * be the last method called on that list before destroying it.
     */
    void clearPluginDescrs(std::list<GWEN_PLUGIN_DESCRIPTION*> &l);

    int activateProvider(const char *pname);
    int deactivateProvider(const char *pname);
    std::list<std::string> getActiveProviders();


    /*@}*/


    /** @name Functions Used by Backends
     *
     */
    /*@{*/
    /**
     * This is used by backends to store their specific data with AqBanking.
     */
    GWEN_DB_NODE *getProviderData(const AB_PROVIDER *pro);
    /*@}*/


    /** @name Enqueueing, Dequeueing and Executing Jobs
     *
     * Enqueued jobs are preserved across shutdowns. As soon as a job has been
     * sent to the appropriate backend it will be removed from the queue.
     * Only those jobs are saved/reloaded which have been enqueued but never
     * presented to the backend. This means after calling
     * @ref AB_Banking_ExecuteQueue only those jobs are still in the queue which
     * have not been processed (e.g. because they belonged to a second backend
     * but the user aborted while the jobs for a first backend were in process).
     */
    /*@{*/
    /**
     * Enqueues a job. This function does not take over the ownership of the
     * job. However, this function makes sure that the job will not be deleted
     * as long as it is in the queue (by calling @ref AB_Job_Attach).
     * So it is safe for you to call @ref AB_Job_free on an enqueued job directly
     * after enqueuing it (but it doesn't make much sense since you would not be able to
     * check for a result).
     *
     */
    int enqueueJob(AB_JOB *j);

    /**
     * Removes a job from the queue. This function does not free the given
     * job, the caller still is the owner.
     * Dequeued jobs however are NOT preserved across shutdowns.
     */
    int dequeueJob(AB_JOB *j);

    /**
     * This function sends all jobs in the queue to their corresponding backends
     * and allows that backend to process it.
     * If the user did not abort or there was no fatal error the queue is
     * empty upon return. You can verify this by calling
     * @ref AB_Banking_GetEnqueuedJobs.
     */
    int executeQueue();

    /**
     * Returns the list of currently enqueued jobs. If the queue is empty
     * NULL is returned.
     */
    std::list<AB_JOB*> getEnqueuedJobs();
    /*@}*/



  };


} /* namespace */


#endif /* AQ_BANKING_CPP_H */


