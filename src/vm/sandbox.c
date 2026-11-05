/*
 * Sandbox support
 *
 * Copyright 2026 Pablo Botella
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file LICENSE.txt.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA (or visit https://www.gnu.org/licenses/).
 *
 * As a special exception, the Harbour Project gives permission for
 * additional uses of the text contained in its release of Harbour.
 *
 * The exception is that, if you link the Harbour libraries with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the Harbour library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by the Harbour
 * Project under the name Harbour.  If you copy code from other
 * Harbour Project or Free Software Foundation releases into a copy of
 * Harbour, as the General Public License permits, the exception does
 * not apply to the code that you add in this way.  To avoid misleading
 * anyone as to the status of such modified files, you must delete
 * this exception notice from them.
 *
 * If you write modifications of your own for Harbour, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 *
 */

#if defined( HB_VM_SANDBOX )

#define HB_STACK_PRELOAD

#include "hbvmopt.h"
#include "hbapi.h"
#include "hbstack.h"
#include "hbapierr.h"
#include "hbapiitm.h"


/* constructor */
PHB_SANDBOX hb_sandboxNew( PHB_SANDBOX hParent )
{
   PHB_SANDBOX pSandbox = ( PHB_SANDBOX ) hb_xgrab( sizeof( struct _HB_SANDBOX ) );
   memset( pSandbox, 0, sizeof( struct _HB_SANDBOX ) );

   pSandbox->hParent = hParent;
   return pSandbox;
}
/* destructor */
void hb_sandboxDestroy( PHB_SANDBOX pSandbox )
{
   if( pSandbox )
   {
      hb_sandboxSetName( pSandbox , NULL );
      hb_xfree( pSandbox );
      pSandbox = NULL;
   }
}

void hb_sandboxSetName( PHB_SANDBOX pSandbox, const char * szName )
{
   if( ! pSandbox )
   {
      return;
   }
   if( pSandbox->pSandBoxName )
   {
      hb_xfree( pSandbox->pSandBoxName );
      pSandbox->pSandBoxName = NULL;
   }
   if( szName && szName[ 0 ] )
   {
      pSandbox->pSandBoxName = hb_strdup( szName );
   }
}
char * hb_sandboxGetName( PHB_SANDBOX pSandbox )
{
   return pSandbox ? pSandbox->pSandBoxName : NULL;
}


PHB_SANDBOX hb_sandboxGetParent( PHB_SANDBOX hSandbox )
{
   return hSandbox ? hSandbox->hParent : NULL;
}

void hb_vmSandboxEnvelopeClean( void )
{
   HB_STACK_TLS_PRELOAD
   PHB_SANDBOX hSandbox = hb_stack.hSandbox;

   if( hSandbox )
   {
      hb_stack.hSandbox = hb_sandboxGetParent( hSandbox );
      hb_sandboxDestroy( hSandbox );
   }
}



HB_FUNC( HB_CREATESANDBOX )
{
   HB_STACK_TLS_PRELOAD
   PHB_ITEM pRecover;

   /* hb_CreateSandbox() must be called immediately after BEGIN SEQUENCE.
    * The recover envelope must be right below the current stack position.
    * The current function call pushes its own frame on top, so we look
    * at nRecoverBase which points to the SEQUENCE envelope.
    */
   if( hb_stack.nRecoverBase == 0 )
   {
      hb_errInternal( HB_EI_ERRUNRECOV,
                       "hb_CreateSandbox() called without BEGIN SEQUENCE", NULL, NULL );
      return;
   }

   pRecover = hb_stackItem( hb_stack.nRecoverBase - 1 );

   if( pRecover->type != HB_IT_RECOVER )
   {
      hb_errInternal( HB_EI_ERRUNRECOV,
                       "hb_CreateSandbox(): invalid SEQUENCE envelope", NULL, NULL );
      return;
   }

   /* Check if this SEQUENCE already has a sandbox */
   if( pRecover->item.asRecover.flags & HB_SEQ_HASSANDBOX )
   {
      hb_errInternal( HB_EI_ERRUNRECOV,
                       "hb_CreateSandbox(): SEQUENCE already has a sandbox", NULL, NULL );
      return;
   }

   /* Mark the SEQUENCE envelope as having a sandbox */
   pRecover->item.asRecover.flags |= HB_SEQ_HASSANDBOX;

   /* Create and push the sandbox via constructor */
   hb_stack.hSandbox = hb_sandboxNew( hb_stack.hSandbox );

   /* Optional name parameter */
   if( hb_pcount() >= 1 && HB_ISCHAR( 1 ) )
   {
      hb_sandboxSetName( hb_stack.hSandbox, hb_parc( 1 ) );
   }
}

HB_FUNC( HB_SANDBOXGETNAME )
{
   HB_STACK_TLS_PRELOAD

   hb_retc( hb_sandboxGetName( hb_stack.hSandbox ) );
}

#endif /* HB_VM_SANDBOX */
