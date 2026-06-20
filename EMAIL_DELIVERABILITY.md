# Email Deliverability Guide

## Why Emails Land in Spam

Spam filters evaluate multiple factors:
1. **Missing SPF/DKIM/DMARC records** (DNS authentication)
2. **No plain-text alternative** (only HTML)
3. **Generic "noreply@" addresses** without proper DNS
4. **Missing email headers** (Message-ID, Date, Reply-To)
5. **Low sender reputation** (new domain/IP)
6. **Poor email content** (spammy words, too many links)

## What We Fixed in Code

✅ **Plain-text alternative**: All emails now include both HTML and plain-text versions
✅ **Proper email headers**: Message-ID, Date, Reply-To, X-Mailer added
✅ **Reply-To header**: Uses admin email for replies
✅ **Multipart MIME**: RFC-compliant email structure

## What You MUST Configure (DNS/Domain)

### 1. Use Your Own Domain (mix.stoersender.ch)

**❌ DON'T**: Use gmail.com, yahoo.com, or free email providers as "From" address
**✅ DO**: Use `noreply@stoersender.ch` or `reamark@stoersender.ch`

### 2. Configure SPF Record

SPF (Sender Policy Framework) tells receiving servers which IPs can send email for your domain.

**Add this TXT record to your DNS** (`stoersender.ch`):

```
Type: TXT
Name: @
Value: v=spf1 ip4:YOUR_DISKSTATION_IP include:_spf.google.com ~all
```

Replace `YOUR_DISKSTATION_IP` with your DiskStation's public IP address.

If using Gmail SMTP: `v=spf1 include:_spf.google.com ~all`
If using SendGrid: `v=spf1 include:sendgrid.net ~all`
If using Mailgun: `v=spf1 include:mailgun.org ~all`

### 3. Configure DKIM Signature

DKIM (DomainKeys Identified Mail) cryptographically signs your emails.

#### For Gmail SMTP:
1. Go to Google Admin Console → Apps → Gmail → Authenticate email
2. Generate new DKIM key for `stoersender.ch`
3. Add the provided DNS TXT record

#### For SendGrid:
1. SendGrid Dashboard → Settings → Sender Authentication → Authenticate Domain
2. Follow wizard, add provided DNS records (CNAME records for DKIM)

#### For Mailgun:
1. Mailgun Dashboard → Sending → Domains → Add Domain
2. Add the MX, TXT (SPF), and CNAME (DKIM) records shown

### 4. Configure DMARC Policy

DMARC tells receiving servers what to do if SPF/DKIM fail.

**Add this TXT record**:

```
Type: TXT
Name: _dmarc
Value: v=DMARC1; p=quarantine; rua=mailto:YOUR_EMAIL@stoersender.ch; pct=100; adkim=r; aspf=r
```

This policy:
- `p=quarantine`: Quarantine (spam folder) emails that fail authentication
- `rua=mailto:...`: Send aggregate reports to your email
- `pct=100`: Apply policy to 100% of emails
- `adkim=r`, `aspf=r`: Relaxed alignment (allows subdomains)

### 5. Configure Reverse DNS (PTR Record)

If your DiskStation has a static IP, set up reverse DNS (PTR) record to point back to `mail.stoersender.ch`.

**Contact your ISP** to configure reverse DNS for your IP address.

### 6. Set Up Return-Path / Bounce Handling

For SMTP, configure your mail server's return-path to match your domain.

In ReaMark settings, set:
- **From Address**: `reamark@stoersender.ch`
- **From Name**: `Störsender ReaMark`
- **Admin Email** (Reply-To): Your real email (e.g., `frank@stoersender.ch`)

## Recommended SMTP Providers

### Option 1: Gmail SMTP (Free, Simple)
- **Pros**: Easy setup, good reputation
- **Cons**: 500 emails/day limit, requires app password
- **Setup**:
  1. Enable 2FA on Google account
  2. Create App Password: https://myaccount.google.com/apppasswords
  3. Use `smtp.gmail.com:587` with TLS
  4. **From Address MUST match your Gmail address** (e.g., `frank@stoersender.ch` if using Google Workspace)

### Option 2: SendGrid (Free Tier: 100/day)
- **Pros**: Professional, excellent deliverability, DKIM/SPF pre-configured
- **Cons**: Requires account signup
- **Setup**:
  1. Sign up at sendgrid.com
  2. Authenticate your domain (wizard adds DNS records)
  3. Create API key
  4. Use API key in ReaMark settings

### Option 3: Mailgun (Free Tier: 5,000/month)
- **Pros**: Generous free tier, good for transactional emails
- **Cons**: Requires domain verification
- **Setup**:
  1. Sign up at mailgun.com
  2. Add domain, configure DNS records
  3. Create API key
  4. Use API key + domain in ReaMark settings

### Option 4: Self-Hosted SMTP (Advanced)
- **Pros**: Full control, no third-party
- **Cons**: Complex setup, IP reputation issues, maintenance
- **Not recommended** unless you have email server experience

## Testing Your Configuration

### 1. Check DNS Records
```bash
# Check SPF
dig TXT stoersender.ch

# Check DKIM (after configuring)
dig TXT default._domainkey.stoersender.ch

# Check DMARC
dig TXT _dmarc.stoersender.ch
```

### 2. Send Test Email from ReaMark
1. Go to Admin → Settings → Email
2. Configure provider + credentials
3. Click "Test Email"
4. Check inbox AND spam folder

### 3. Use Email Deliverability Testers
- **mail-tester.com**: Sends test email, scores deliverability (aim for 10/10)
- **mxtoolbox.com/deliverability**: Checks DNS records
- **dmarcian.com**: DMARC analyzer

### 4. Check Email Headers
When you receive the test email, view full headers (Gmail: Show original). Look for:
- ✅ `SPF: PASS`
- ✅ `DKIM: PASS`
- ✅ `DMARC: PASS`
- ✅ `Authentication-Results: ... spf=pass dkim=pass`

## Common Issues & Solutions

### Issue: Emails still going to spam
**Solutions**:
1. Verify SPF/DKIM/DMARC records are correct (use mail-tester.com)
2. Use a reputable SMTP provider (SendGrid/Mailgun) instead of self-hosted
3. Warm up your domain (send gradually increasing volume over weeks)
4. Avoid spam trigger words ("free", "click here", excessive exclamation marks)
5. Ask recipients to mark emails as "Not Spam" and add to contacts

### Issue: Gmail blocks emails from DiskStation IP
**Solutions**:
1. Use Gmail SMTP instead of direct sending
2. Use SendGrid/Mailgun (they have pre-warmed IPs)
3. Set up reverse DNS (PTR record)

### Issue: "Authentication failed" error
**Solutions**:
1. For Gmail: Use App Password (not account password)
2. Check username/password are correct
3. Verify TLS/SSL settings match provider requirements

### Issue: SPF record not found
**Solutions**:
1. DNS propagation can take 24-48 hours
2. Verify record is on root domain (`stoersender.ch`), not subdomain
3. Check for typos in DNS record

## Recommended Setup for Störsender

Given your infrastructure (Synology DiskStation, mix.stoersender.ch), here's the recommended configuration:

### DNS Records to Add
```
# SPF (choose one based on provider)
stoersender.ch.    TXT    "v=spf1 include:sendgrid.net ~all"

# DMARC
_dmarc.stoersender.ch.    TXT    "v=DMARC1; p=quarantine; rua=mailto:frank@stoersender.ch"

# DKIM (will be provided by SendGrid/Mailgun after domain verification)
```

### ReaMark Email Settings
```
Provider: SendGrid (or Mailgun)
From Address: reamark@stoersender.ch
From Name: Störsender ReaMark
Admin Email (Reply-To): frank@stoersender.ch (or your real email)
```

### Why SendGrid/Mailgun?
- ✅ Pre-warmed IP addresses (good reputation)
- ✅ Automatic DKIM signing
- ✅ Bounce/complaint handling
- ✅ Generous free tier
- ✅ No need to manage mail server
- ✅ Better deliverability than self-hosted

## Email Template Best Practices

### ✅ DO:
- Use plain language, avoid marketing jargon
- Include clear unsubscribe link (optional for transactional emails)
- Keep HTML simple (inline CSS, avoid complex layouts)
- Include plain-text version (now automatic)
- Use branded "From" name (e.g., "Störsender ReaMark")
- Include company address in footer (builds trust)

### ❌ DON'T:
- Use "noreply@" if possible (better: "reamark@" with Reply-To)
- Use ALL CAPS in subject lines
- Include shortened links (bit.ly, etc.) — use full URLs
- Embed large images (use hosted images with alt text)
- Use spam trigger words ("Free", "Act now", "Limited time")

## Monitoring & Maintenance

### Regular Checks
1. **Weekly**: Check spam folder for legitimate emails
2. **Monthly**: Review DMARC aggregate reports (if configured)
3. **Quarterly**: Test deliverability with mail-tester.com
4. **Yearly**: Rotate API keys/passwords

### Metrics to Track
- **Bounce rate**: Should be <5% (higher = list quality issue)
- **Spam complaint rate**: Should be <0.1%
- **Deliverability rate**: Should be >95%

## Support Resources

- **SendGrid Docs**: https://docs.sendgrid.com/
- **Mailgun Docs**: https://documentation.mailgun.com/
- **Gmail SMTP**: https://support.google.com/mail/answer/7126229
- **DMARC Guide**: https://dmarc.org/overview/
- **Email Deliverability**: https://www.validity.com/resource-center/email-deliverability-guide/

---

**Need help?** Check the [ReaMark documentation](./CLAUDE.md) or contact the developer.
