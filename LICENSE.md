# Aetheris Open License (AOL) v1.0

**Copyright (c) 2026 The Aetheris Foundation and Contributors**

> **Notice:** This is a custom license written for the Aetheris project. It has not been reviewed by a lawyer. Have qualified legal counsel review it before relying on it, and replace the bracketed placeholders (such as the contact address) with real values.

---

## 1. Definitions

* **"Software"** means the Aetheris compiler (`aethc`), interpreter (`aethi`), toolchain (`aeth`), linker (`aethl`), language server (`aeth-lsp`), runtime (`aethrt`), standard library (`std`), and associated documentation and specifications, in source and binary form.
* **"Contributor"** means any person or entity that submits material to the Software.
* **"Contribution"** means any work of authorship intentionally submitted to the Foundation for inclusion in the Software.
* **"Derivative Work"** means any modification of, or work based on, the Software, excluding Program Output.
* **"Program Output"** means any code, binaries, bytecode, WebAssembly modules, assembly, GPU kernels, or other artifacts produced by running the Software on your own source code.
* **"You"** means the individual or legal entity exercising permissions under this License.
* **"Foundation"** means the Aetheris Foundation or its successors.

---

## 2. Grant of Copyright License

Subject to the terms of this License, each Contributor grants You a perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable (except as stated in Section 6) license to use, copy, modify, merge, publish, distribute, sublicense, and sell copies of the Software and Derivative Works, in source or binary form, for any purpose, including commercial use.

## 3. Grant of Patent License

Each Contributor grants You a perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable (except as stated in Section 6) patent license to make, have made, use, offer to sell, sell, import, and otherwise transfer the Software, where such license applies only to those patent claims necessarily infringed by that Contributor's Contribution alone or by combination of their Contribution with the Software.

## 4. Program Output

**Program Output is yours.** The Foundation and Contributors claim no ownership of, and impose no license restrictions on, code you write in Aetheris or artifacts the Software generates from it. Output may be licensed in any way You choose, including proprietary licenses.

This exception does not extend to portions of the Standard Library or Runtime that are copied into Program Output (for example, statically linked `std` or `aethrt` code). Those portions remain covered by this License, which only requires that You retain the notice in Section 5(c) in your documentation or an accompanying NOTICE file.

## 5. Conditions

When You redistribute the Software or a Derivative Work, in source or binary form, You must:

* **(a)** Include a copy of this License.
* **(b)** Mark modified files with prominent notices stating that You changed them.
* **(c)** Retain all copyright, patent, trademark, and attribution notices from the original Software.
* **(d)** Not remove or alter the `AETHERIS_NOTICE` file, if present, and reproduce its contents in your own NOTICE file.

Redistribution of binaries built from the unmodified Software only requires including the notice described in 5(c) in documentation or an "About" screen.

## 6. Patent Retaliation and Termination

If You initiate patent litigation (including a cross-claim or counterclaim) alleging that the Software or a Contribution infringes a patent, the patent licenses granted to You under Section 3 terminate on the date such litigation is filed.

If You materially breach this License and do not cure the breach within **30 days** of written notice or of becoming aware of it, Your rights under this License terminate. Rights are reinstated if You cure the breach.

## 7. Trademarks and Branding

This License does not grant permission to use the names "Aetheris," "aethc," "aethi," "aeth," the `.aeth` brand marks, or the Foundation's logos, except for reasonable and customary use in describing the origin of the Software and in reproducing the contents of the notice files.

Modified versions that are distributed must not be presented as the official Aetheris distribution. Use of the phrase "compatible with Aetheris" is permitted if the claim is accurate.

## 8. Contributions

Unless You explicitly state otherwise, any Contribution You submit for inclusion in the Software is submitted under the terms of this License, without additional terms or conditions. You represent that You have the right to submit the Contribution under this License. Contributors retain copyright in their Contributions.

## 9. Third-Party Components

The Software may include or depend on third-party components that are licensed separately. Those components are listed in `THIRD_PARTY_NOTICES.md` and remain under their respective licenses. Nothing in this License changes those terms.

## 10. Disclaimer of Warranty

THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE, AND NON-INFRINGEMENT. YOU ASSUME ALL RISK ARISING FROM USE OF THE SOFTWARE.

**Safety-critical use:** The Software is not designed or certified for use in environments where failure could lead to death, personal injury, or severe physical or environmental damage (such as medical life support, nuclear facilities, or aircraft control) unless You independently validate and certify it for that purpose.

## 11. Limitation of Liability

TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL ANY CONTRIBUTOR OR THE FOUNDATION BE LIABLE FOR ANY INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES, OR FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT, OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR ITS USE OR OTHER DEALINGS IN THE SOFTWARE.

## 12. Export Compliance

You are responsible for complying with all applicable export control and sanctions laws when You use or redistribute the Software, including cryptographic functionality in `std.crypto`.

## 13. General

* **Severability**: If any provision is held unenforceable, the remaining provisions remain in effect and the unenforceable provision is modified to the minimum extent necessary.
* **No Waiver**: Failure to enforce a provision is not a waiver of that provision.
* **Entire Agreement**: This License is the complete agreement regarding the Software, except for separately licensed third-party components.
* **Governing Law**: [Specify jurisdiction, e.g. "the laws of the State of Delaware, USA"].
* **Future Versions**: The Foundation may publish revised versions of this License. You may continue under v1.0 or elect to use any later version.

---

## Appendix: Applying This License

Add the following header to source files:

```text
Copyright (c) 2026 The Aetheris Foundation and Contributors
Licensed under the Aetheris Open License, Version 1.0.
See LICENSE.md in the project root for the full license text.
```

Questions about this License: [license@your-domain.example]
