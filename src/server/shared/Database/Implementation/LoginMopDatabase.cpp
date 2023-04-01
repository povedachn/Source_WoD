////////////////////////////////////////////////////////////////////////////////
//
//  Inna Hoover Brown-STUDIO
//  Copyright 2023 Inna Hoover Brown-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "LoginMopDatabase.h"

void LoginMopDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_LOGINMOPDATABASE_STATEMENTS);

    //PREPARE_STATEMENT(LOGINMOP_SEL_TRANSFER, "SELECT `id`, `account_ashran`, `dump` FROM transfer_ashran WHERE `realm_ashran` = ? AND state = 1", CONNECTION_SYNCH);
}
