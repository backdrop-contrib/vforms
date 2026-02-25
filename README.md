# Node Form Variants (vforms)

Node Form Variants (vforms) lets you create and manage multiple **edit-form layouts** (“variants”) per content type. Each variant can include/exclude fields, change ordering, and optionally organize fields into groups.

This module lets you define multiple “layouts” for the node edit form of a content type, then choose which layout to use when editing.
A concrete use-case:
You have a content type like Work Order with 40+ fields. Different people touch different subsets of those fields.
- Intake variant (front desk / CSR): only customer, contact, short description, priority.
- Production variant (shop lead): materials, routing, quantities, due dates.
- QA variant: inspection fields, sign-off, attachments.
- Admin variant: everything, including rarely-used metadata fields.

Without vforms, everyone edits the same long form, leading to:
- slower data entry (scrolling and hunting),
- more mistakes (editing fields they shouldn’t touch),
- poorer usability on mobile/tablet.

With vforms, you create those variants once, and then provide links (including Views “edit using variant” links) that open the right form for the right workflow. It’s essentially “multiple tailored edit screens for the same content type,” without creating duplicate content types or writing custom form-alter code per workflow.

## Features

- Create any number of form variants per node type (bundle).
- Reorder fields and “extra fields” (form elements provided by modules).
- Show/hide fields per variant.
- Create simple groups and drag fields into them.
- Control group fieldset behavior per group:
  - **open** (collapsible, initially open)
  - **closed** (collapsible, initially closed)
  - **locked open** (non-collapsible)
- Stores configuration in Backdrop’s config system (exportable/importable).
- Provides a **Form variant edit link** Views field

## Requirements

- Backdrop CMS 1.x
- Core jQuery UI dialog library is used for the “floaty dialog” variant editor where applicable.

## Installation

- Install this module using the official Backdrop CMS instructions at
  https://docs.backdropcms.org/documentation/extend-with-modules.
- Enable the module at:  
   **Administration → Functionality → List modules**

## Permissions

Grant permissions as needed at:  
**Administration → People → Permissions**

Look for permissions under the `vforms` module section.

## Configuration / Where to find it

For each content type:

- Go to: **Administration → Structure → Content types**
- Click **Form variants** for the type.

Variant edit path pattern:

`admin/structure/types/manage/%type/vforms/%variant/edit`

## Configuration storage (database impact)

vforms does **not** create custom database tables.

Variants are stored as Backdrop configuration objects, typically:

- `vforms.node.<bundle>`

Example:
- `vforms.node.article`

These config objects can be exported/imported via **Configuration Management**.

## Theming notes (dialog)

The floating editor dialog uses Backdrop’s AJAX dialog (jQuery UI). The dialog “chrome” is jQuery UI; the dialog *content* is rendered with whichever theme Backdrop applies to that route (site theme vs admin theme).

If your site is configured to “Use the administration theme when editing or creating content”, the dialog content may inherit the admin theme even on non-`/admin` paths.

## Project status

This module is in early (alpha) stage. APIs and UI may change.

## Current Maintainers

- [ericfoy](https://github.com/ericfoy)

## Credits

- This module was created for Backdrop by [ericfoy](https://github.com/ericfoy)
- Sponsored by [Perideo LLC](https://perideo.com).

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.
