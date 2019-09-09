//
// UC_BreadCrumb.xaml.h
// Declaration of the UC_BreadCrumb class
//

#pragma once

#include "UC_BreadCrumb.g.h"
#include "Auxiliary.h"

namespace EggEditor
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class UC_BreadCrumb sealed
	{
		Platform::Object ^ breadCrumbRef;
	public:
		UC_BreadCrumb(Platform::Object^ bcRef);

		property Platform::Object ^ BreadCrumbRef {
			Platform::Object ^ get() {
				return breadCrumbRef;
			}
		}

		property BreadCrumbDataContext ^ TypedDataContext {
			BreadCrumbDataContext ^ get() {
				return (BreadCrumbDataContext ^)DataContext;
			}
		}
	};
}
